using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class playerObject
{
    public GameObject Player;
    public otherEllenController Controller;
}

public class GameManager : MonoBehaviour
{
    public Canvas HPCanvas;

    protected static GameManager s_Instance;
    public NetworkManager Network;
    public GameObject[] prefabsArry;
    public Dictionary<System.UInt64, playerObject> playerDic;
    public Dictionary<System.UInt64, GameObject> hpDic;
    
    public GameObject Player;
    public Gamekit3D.PlayerController Controller;
    System.UInt64 Index;

    

    public struct hitMsg
    {
        public System.UInt64 targetIndex;
        public int                 Damage;
        public Vector3          Position;
    }
    public static GameManager Instance
    {
        get
        {
            if (s_Instance != null)
                return s_Instance;

            s_Instance = FindObjectOfType<GameManager>();

            if (s_Instance != null)
                return s_Instance;

            GameManager Game = Resources.Load<GameManager>("GameManager");
            s_Instance = Instantiate(Game);

            return s_Instance;
        }
    }

    private void Awake()
    {
        Network = NetworkManager.Instance;
        if (Instance != this)
        {
            Destroy(gameObject);
            return;
        }
        DontDestroyOnLoad(gameObject);
    }

    // Use this for initialization
    void Start()
    {
        if (Instance.playerDic == null)
            Instance.playerDic = new Dictionary<System.UInt64, playerObject>();
        if (Instance.hpDic == null)
            Instance.hpDic = new Dictionary<ulong, GameObject>();

        Instance.HPCanvas = GameObject.Find("UICanvases").GetComponent<Canvas>();
        NetworkManager.Instance.GAME_LOGIN();
    }

    public void Game_createChar(byte[] _data, bool _controlType)
    {
        byte characterType;
        float xPos, zPos, yForward, wForward;
        int Level, maxHP, currentHP;
        string nickName;
        System.UInt64 charIndex;

        charIndex = System.BitConverter.ToUInt64(_data, 7);
        characterType = _data[15];
        nickName = System.Text.Encoding.Unicode.GetString(_data, 16, 20);
        xPos = System.BitConverter.ToSingle(_data, 36);
        zPos = System.BitConverter.ToSingle(_data, 40);
        yForward = System.BitConverter.ToSingle(_data, 44);
        wForward = System.BitConverter.ToSingle(_data, 48);
        Level = System.BitConverter.ToInt16(_data, 52);
        maxHP = System.BitConverter.ToInt32(_data, 54);
        currentHP = System.BitConverter.ToInt32(_data, 58);

        Vector3 Position = new Vector3(xPos, 0.5f, zPos);
        Quaternion transformForward = new Quaternion(0, yForward, 0, wForward);

        proc_createChar(charIndex, characterType, nickName, Position, transformForward, _controlType,Level,maxHP, currentHP);
    }

    void proc_createChar(System.UInt64 _Index, byte _charType, string _nickName, Vector3 _Position, Quaternion _transformForward, bool _controlType, int _Level, int _maxHP, int _currentHP)
    {
        GameObject newPlayer = Instantiate(Instance.prefabsArry[_charType], _Position, _transformForward);
        playerObject player = new playerObject();

        GameObject nameObj = Instantiate(Resources.Load("NameObject")) as GameObject;
        nameObj.transform.parent = newPlayer.transform;
        playerName name = nameObj.AddComponent<playerName>();
        name.setPlayer(newPlayer, _nickName);

        GameObject hpBar = Instantiate(Resources.Load("HPBar")) as GameObject;
        hpBar.transform.parent = Instance.HPCanvas.transform;

        hpController hpCntler = newPlayer.AddComponent<hpController>();

        if (!_controlType)
        {
            Destroy(newPlayer.GetComponent<Gamekit3D.PlayerController>());
            Destroy(newPlayer.GetComponent<PlayerInput>());
            player.Controller = newPlayer.AddComponent<otherEllenController>();
            player.Controller.setIndex(_Index);
            player.Player = newPlayer;
            player.Controller.setPlayerData(_Level, _maxHP, _currentHP, hpCntler, hpBar.GetComponent<Slider>());
            Instance.playerDic.Add(_Index, player);
            Instance.hpDic.Add(_Index, hpBar);
        }
        else
        {
            newPlayer.GetComponent<PlayerInput>().enabled = true;
            Instance.Controller = newPlayer.GetComponent<Gamekit3D.PlayerController>();
            Player = newPlayer;
            Instance.Index = _Index;
            Instance.Controller.setPlayerData(_Level, _maxHP,_currentHP,hpCntler, hpBar.GetComponent<Slider>());
            Instance.StartCoroutine(Gamekit3D.ScreenFader.FadeSceneIn());
        }
    }

    public void Game_deleteChar(byte[] _data)
    {
        System.UInt64 Index;

        Index = System.BitConverter.ToUInt64(_data, 7);

        proc_deleteChar(Index);
    }

    void proc_deleteChar(System.UInt64 _Index)
    {
        if (Instance.playerDic.ContainsKey(_Index).Equals(true))
        {
            Object.Destroy(Instance.playerDic[_Index].Player);
            Instance.playerDic.Remove(_Index);
        }
        if(Instance.hpDic.ContainsKey(_Index).Equals(true))
        {
            GameObject Val = Instance.hpDic[_Index];
            Instance.hpDic.Remove(_Index);
            Destroy(Val);

        }
    }

    public void Game_moveReq(Vector3 _Direction, Quaternion _transformForward, Vector3 _currentPosition)
    {
        
        byte[] Type = System.BitConverter.GetBytes((short)Protocol.game_move_req);
        byte[] Index = System.BitConverter.GetBytes(Instance.Index);
        byte[] xDir = System.BitConverter.GetBytes(_Direction.x);
        byte[] zDir = System.BitConverter.GetBytes(_Direction.z);
        byte[] xForward = System.BitConverter.GetBytes(_transformForward.y);
        byte[] zForward = System.BitConverter.GetBytes(_transformForward.w);
        byte[] xPos = System.BitConverter.GetBytes(_currentPosition.x);
        byte[] zPos = System.BitConverter.GetBytes(_currentPosition.z);
        
        byte[] msg = new byte[39];
        System.Array.Copy(Type, 0, msg, 0, Type.Length);
        System.Array.Copy(Index, 0, msg, 2, Index.Length);
        System.Array.Copy(xDir, 0, msg, 10, xDir.Length);
        System.Array.Copy(zDir, 0, msg, 14, zDir.Length);
        System.Array.Copy(xForward, 0, msg, 18, xForward.Length);
        System.Array.Copy(zForward, 0, msg, 22, zForward.Length);
        System.Array.Copy(xPos, 0, msg, 26, xPos.Length);
        System.Array.Copy(zPos, 0, msg, 30, zPos.Length);

        NetworkManager.Instance.sendMsg(msg);
    }

    public void Game_moveRes(byte[] _data)
    {
        System.UInt64 Index;
        float xDir, zDir;
        float xForward, zForward;

        Index = System.BitConverter.ToUInt64(_data, 7);
        xDir = System.BitConverter.ToSingle(_data, 15);
        zDir = System.BitConverter.ToSingle(_data, 19);
        xForward = System.BitConverter.ToSingle(_data, 23);
        zForward = System.BitConverter.ToSingle(_data, 27);

        Vector3 Direction = new Vector3(xDir, -1f, zDir);
        Quaternion transformForward = new Quaternion(0, xForward, 0, zForward);

        proc_Move(Index, Direction, transformForward);
    }

    void proc_Move(System.UInt64 _Index, Vector3 _Direction, Quaternion _transformForward)
    {
        if (Instance.playerDic.ContainsKey(_Index).Equals(true))
            Instance.playerDic[_Index].Controller.setMove(_transformForward, true);
    }

    public void Game_stopReq(Quaternion _transformForward, Vector3 _currentPosition)
    {
        byte[] Type = System.BitConverter.GetBytes((short)Protocol.game_stop_req);
        byte[] Index = System.BitConverter.GetBytes(Instance.Index);
        byte[] xForward = System.BitConverter.GetBytes(_transformForward.y);
        byte[] zForward = System.BitConverter.GetBytes(_transformForward.w);
        byte[] xPos = System.BitConverter.GetBytes(_currentPosition.x);
        byte[] zPos = System.BitConverter.GetBytes(_currentPosition.z);
        
        byte[] msg = new byte[39];
        System.Array.Copy(Type, 0, msg, 0, Type.Length);
        System.Array.Copy(Index, 0, msg, 2, Index.Length);
        System.Array.Copy(xForward, 0, msg, 10, xForward.Length);
        System.Array.Copy(zForward, 0, msg, 14, zForward.Length);
        System.Array.Copy(xPos, 0, msg, 18, xPos.Length);
        System.Array.Copy(zPos, 0, msg, 22, zPos.Length);

        NetworkManager.Instance.sendMsg(msg);
    }

    void proc_Stop(System.UInt64 _Index, Vector3 _Direction, Quaternion _transformForward, Vector3 _position)
    {
        if (Instance.playerDic.ContainsKey(_Index).Equals(true))
        {
            Instance.playerDic[_Index].Controller.setMove(_transformForward, false);
            float distance = (playerDic[_Index].Player.transform.position - _position).sqrMagnitude;
            if (distance > 5f)
                proc_sync(_Index, _transformForward, _position);
            
        }
    }

    public void Game_stopRes(byte[] _data)
    {
        System.UInt64 Index;
        float xForward, zForward, xPos, zPos;

        Index = System.BitConverter.ToUInt64(_data, 7);
        xForward = System.BitConverter.ToSingle(_data, 15);
        zForward = System.BitConverter.ToSingle(_data, 19);
        xPos = System.BitConverter.ToSingle(_data, 23);
        zPos = System.BitConverter.ToSingle(_data, 27);


        Vector3 Direction = new Vector3(0, -1, 0);
        Vector3 Position = new Vector3(xPos,1f, zPos);
        Quaternion transformForward = new Quaternion(0, xForward, 0, zForward);

        proc_Stop(Index, Direction, transformForward, Position);
    }

    public void Game_syncRes(byte[] _data)
    {

        System.UInt64 Index;
        float xPos, zPos;
        float xForward, zForward;

        Index = System.BitConverter.ToUInt64(_data, 7);
        xPos = System.BitConverter.ToSingle(_data, 15);
        zPos = System.BitConverter.ToSingle(_data, 19);
        xForward = System.BitConverter.ToSingle(_data, 23);
        zForward = System.BitConverter.ToSingle(_data, 27);

        Vector3 Position = new Vector3(xPos, 1, zPos);
        Quaternion transformForward = new Quaternion(0, xForward, 0, zForward);

        proc_sync(Index, transformForward, Position);
    }

    void proc_sync(System.UInt64 _Index, Quaternion _transformForward, Vector3 _Position)
    {
        if (Instance.playerDic.ContainsKey(_Index).Equals(false))
            return;
        Instance.playerDic[_Index].Player.transform.position = _Position;
        Instance.playerDic[_Index].Player.transform.rotation = _transformForward;
    }

    public void Game_attackReq(Quaternion _Rotation)
    {
        byte[] Type = System.BitConverter.GetBytes((short)Protocol.game_attack_req);
        byte[] Index = System.BitConverter.GetBytes(Instance.Index);
        byte[] xRotation = System.BitConverter.GetBytes(_Rotation.y);
        byte[] zRotation = System.BitConverter.GetBytes(_Rotation.w);

        byte[] msg = new byte[18];
        System.Array.Copy(Type, 0, msg, 0, Type.Length);
        System.Array.Copy(Index, 0, msg, 2, Index.Length);
        System.Array.Copy(xRotation, 0, msg, 10, xRotation.Length);
        System.Array.Copy(zRotation, 0, msg, 14, zRotation.Length);

        NetworkManager.Instance.sendMsg(msg);
    }

    public void Game_attackRes(byte[] _data)
    {
        System.UInt64 Index;
        float xRotation, zRotation;

        Index = System.BitConverter.ToUInt64(_data, 7);
        xRotation = System.BitConverter.ToSingle(_data, 15);
        zRotation = System.BitConverter.ToSingle(_data, 19);

        Quaternion Rotation = new Quaternion(0, xRotation, 0, zRotation);
        proc_Attack(Index,Rotation);
    }

    void proc_Attack(System.UInt64 _Index, Quaternion _Rotation)
    {
        if (Instance.playerDic.ContainsKey(_Index).Equals(false))
            return;
        Instance.playerDic[_Index].Controller.setAttack(_Rotation, true);
    }

    public void Game_hitReq(hitMsg _data)
    {
        byte[] Type = System.BitConverter.GetBytes((short)Protocol.game_hit_req);
        byte[] Index = System.BitConverter.GetBytes(Instance.Index);
        byte[] targetIndex = System.BitConverter.GetBytes(_data.targetIndex);
        byte[] xPosition = System.BitConverter.GetBytes(_data.Position.x);
        byte[] zPosition = System.BitConverter.GetBytes(_data.Position.z);
        byte[] Damage = System.BitConverter.GetBytes(_data.Damage);

        byte[] msg = new byte[30];
        System.Array.Copy(Type, 0, msg, 0, Type.Length);
        System.Array.Copy(Index, 0, msg, 2, Index.Length);
        System.Array.Copy(targetIndex, 0, msg, 10, targetIndex.Length);
        System.Array.Copy(xPosition, 0, msg, 18, xPosition.Length);
        System.Array.Copy(zPosition, 0, msg, 22, zPosition.Length);
        System.Array.Copy(Damage, 0, msg, 26, Damage.Length);

        NetworkManager.Instance.sendMsg(msg);
    }

    void proc_Hit(System.UInt64 _targetIndex, Vector3 _position,  int _damage, int _HP)
    {
        if(_targetIndex == Instance.Index)
        {
            Instance.Controller.proc_Damage(_position, _damage, _HP);
        }
        else
        {
            if (Instance.playerDic.ContainsKey(_targetIndex).Equals(true))
                Instance.playerDic[_targetIndex].Controller.proc_Damage(_position, _damage, _HP);                
        }
    }

    public void Game_hitRes(byte[] _data)
    {
        // unsigned __int64 attackerIndex
        // unsigned __int64 damagerIndex
        // int                      damage
        // int                      damagerHP;

        System.UInt64  targetIndex;
        int damage, damagerHP;
        float xPosition, zPosition;

        targetIndex = System.BitConverter.ToUInt64(_data, 7);
        xPosition = System.BitConverter.ToSingle(_data, 15);
        zPosition = System.BitConverter.ToSingle(_data, 19);
        damage = System.BitConverter.ToInt32(_data, 23);
        damagerHP = System.BitConverter.ToInt32(_data, 27);

        Vector3 attackerPosition = new Vector3(xPosition, 0, zPosition);
        proc_Hit(targetIndex, attackerPosition, damage, damagerHP);
    }

    void proc_autoRecovery(ulong _Index, int _recoveryAmount ,int _currentHP)
    {
        if(_Index == Instance.Index)
        {
            Controller.Recovery(_recoveryAmount, _currentHP);
            return;
        }
        if(Instance.playerDic.ContainsKey(_Index).Equals(true))
        {
            Instance.playerDic[_Index].Controller.proc_Recovery(_recoveryAmount, _currentHP);
        }
    }

    public void Game_autoRecovery(byte[] _data)
    {
        ulong Index;
        int recoveryAmount, currentHP;

        Index = System.BitConverter.ToUInt64(_data, 7);
        recoveryAmount = System.BitConverter.ToInt32(_data, 15);
        currentHP = System.BitConverter.ToInt32(_data, 19);

        proc_autoRecovery(Index, recoveryAmount, currentHP);

    }
}
