using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System;
using UnityEngine.SceneManagement;

public class charSet
{
    public string Name { get; set; }
    public string charNo { get; set; }
    public int Level { get; set; }
    public int charType { get; set; }
    
}
public class charSlot
{
    public GameObject Name { get; set; }
    public GameObject Level { get; set; }
}

public class AuthManager : MonoBehaviour {


    protected static AuthManager s_Instance;
    public NetworkManager Network;
    private string charID;
    private int charNo;

    public nowStatus state;

    public GameObject[] prefabsArry;
    public GameObject[] StartUIArry;
    public InputField[] StartInputFieldArry;
    public GameObject[] LobbyUIArry;

    static public charSlot[] Slot;
    static private charSet[] charArry;

    public enum nowStatus
    {
        Lobby = 0, Create, Delete, Start,
    }

    enum LobbyUIList
    {
        createUI = 0,
        selectUI, nameFailUI, createFailUI, deleteFailUI, deleteUI,
    }

    enum StartUIList
    {
        signUI = 0, withDrawalUI, LoginFailUI,
    }

    enum StartInputFieldList
    {
        loginID = 0, loginPASS,
        signID, signPASS,
        withDrawaID, withDrawalPASS,
    }

    public static AuthManager Instance
    {
        get
        {
            if (s_Instance != null)
                return s_Instance;

            s_Instance = FindObjectOfType<AuthManager>();

            if (s_Instance != null)
                return s_Instance;

            AuthManager AuthMng = Resources.Load<AuthManager>("AuthManager");
            s_Instance = Instantiate(AuthMng);

            return s_Instance;
        }
    }

    void Awake()
    {
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
        Network = NetworkManager.Instance;
        state = nowStatus.Start;
        init_startUI();
        init_charArry();
        setSpawn();
    }

    // Update is called once per frame
    void Update() {

    }

    void init_charArry()
    {
        Slot = new charSlot[3];
        int count;
        for (count = 0; count < 3; count++)
            Slot[count] = new charSlot();
        charArry = new charSet[3];
        for (count = 0; count < 3; count++)
            charArry[count] = new charSet();
    }

    void init_startUI()
    {
        if (Instance.state != nowStatus.Start)
            return;
        init_startUIArry();
        init_startInputFieldArry();
    }

    void init_Lobby()
    {
        init_lobbyUIArry();
        init_Prefabs();

    }

    void init_startUIArry()
    {
        Instance.StartUIArry = new GameObject[3];

          Instance.StartUIArry[(int)StartUIList.signUI] = GameObject.Find("SignUpUI");
           Instance.StartUIArry[(int)StartUIList.withDrawalUI] = GameObject.Find("withDrawalUI").gameObject;
           Instance.StartUIArry[(int)StartUIList.LoginFailUI] = GameObject.Find("LoginFail");

        Instance.StartUIArry[(int)StartUIList.signUI].SetActive(false);
        Instance.StartUIArry[(int)StartUIList.withDrawalUI].SetActive(false);
        Instance.StartUIArry[(int)StartUIList.LoginFailUI].SetActive(false);
    }

    void init_startInputFieldArry()
    {
        Instance.StartInputFieldArry = new InputField[6];
        Instance.StartInputFieldArry[(int)StartInputFieldList.loginID] = GameObject.Find("LoginBar").transform.Find("IDField").GetComponent<InputField>();
        Instance.StartInputFieldArry[(int)StartInputFieldList.loginPASS] = GameObject.Find("LoginBar").transform.Find("PassField").GetComponent<InputField>();
        Instance.StartInputFieldArry[(int)StartInputFieldList.signID] = Instance.StartUIArry[(int)StartUIList.signUI].transform.Find("IDField").GetComponent<InputField>();
        Instance.StartInputFieldArry[(int)StartInputFieldList.signPASS] = Instance.StartUIArry[(int)StartUIList.signUI].transform.Find("PassField").GetComponent<InputField>();
        Instance.StartInputFieldArry[(int)StartInputFieldList.withDrawaID] = Instance.StartUIArry[(int)StartUIList.withDrawalUI].transform.Find("IDField").GetComponent<InputField>();
        Instance.StartInputFieldArry[(int)StartInputFieldList.withDrawalPASS] = Instance.StartUIArry[(int)StartUIList.withDrawalUI].transform.Find("PassField").GetComponent<InputField>();
    }

    void init_lobbyUIArry()
    {
        Instance.LobbyUIArry = new GameObject[6];

        Instance.LobbyUIArry[(int)LobbyUIList.createUI] = GameObject.Find("CreateCanvas");
        Instance.LobbyUIArry[(int)LobbyUIList.selectUI] = GameObject.Find("SelectCanvas");
        Instance.LobbyUIArry[(int)LobbyUIList.nameFailUI] = GameObject.Find("nicknameFail");
        Instance.LobbyUIArry[(int)LobbyUIList.createFailUI] = GameObject.Find("createFail");
        Instance.LobbyUIArry[(int)LobbyUIList.deleteFailUI] = GameObject.Find("deleteFail");
        Instance.LobbyUIArry[(int)LobbyUIList.deleteUI] = GameObject.Find("deleteUI");

        Instance.charNameField = GameObject.Find("NameField").GetComponent<InputField>();
        Instance.LobbyUIArry[(int)LobbyUIList.createUI].SetActive(false);
        Instance.LobbyUIArry[(int)LobbyUIList.nameFailUI].SetActive(false);
        Instance.LobbyUIArry[(int)LobbyUIList.createFailUI].SetActive(false);
        Instance.LobbyUIArry[(int)LobbyUIList.deleteFailUI].SetActive(false);
        Instance.LobbyUIArry[(int)LobbyUIList.deleteUI].SetActive(false);
    }

    void init_Prefabs()
    {
        Instance.prefabsArry = new GameObject[2];
        Instance.prefabsArry[(int)charType.Ellen] = Resources.Load("AuthEllen") as GameObject;
        Instance.prefabsArry[(int)charType.Babarian] = Resources.Load("AuthBaba") as GameObject;
    }

    public void signUp()
    {
        if (Instance.StartInputFieldArry[(int)StartInputFieldList.signID].text.Length <= 0 || Instance.StartInputFieldArry[(int)StartInputFieldList.signID].text.Length > 10 || 
            Instance.StartInputFieldArry[(int)StartInputFieldList.signPASS].text.Length <= 0 || Instance.StartInputFieldArry[(int)StartInputFieldList.signPASS].text.Length > 15)
        {
            LoginFailUI(true);
            return;
        }

        byte[] Type = BitConverter.GetBytes((short)Protocol.login_signUp_req);
        byte[] ID = System.Text.Encoding.ASCII.GetBytes(Instance.StartInputFieldArry[(int)StartInputFieldList.signID].text);
        byte[] PASS = System.Text.Encoding.ASCII.GetBytes(Instance.StartInputFieldArry[(int)StartInputFieldList.signPASS].text);

        byte[] msg = new byte[27];
        Array.Copy(Type, 0, msg, 0, Type.Length);
        Array.Copy(ID, 0, msg, 2, ID.Length);
        Array.Copy(PASS, 0, msg, 12, PASS.Length);
        NetworkManager.Instance.sendMsg(msg);
    }

    public void withDrawal()
    {
        if (Instance.StartInputFieldArry[(int)StartInputFieldList.withDrawaID].text.Length <= 0 || Instance.StartInputFieldArry[(int)StartInputFieldList.withDrawaID].text.Length > 10 || 
            Instance.StartInputFieldArry[(int)StartInputFieldList.withDrawalPASS].text.Length <= 0 || Instance.StartInputFieldArry[(int)StartInputFieldList.withDrawalPASS].text.Length > 15)
        {
            LoginFailUI(true);
            return;
        }

        byte[] Type = BitConverter.GetBytes((short)Protocol.login_withDrawal_req);
        byte[] ID = System.Text.Encoding.ASCII.GetBytes(Instance.StartInputFieldArry[(int)StartInputFieldList.withDrawaID].text);
        byte[] PASS = System.Text.Encoding.ASCII.GetBytes(Instance.StartInputFieldArry[(int)StartInputFieldList.withDrawalPASS].text);

        byte[] msg = new byte[27];
        Array.Copy(Type, 0, msg, 0, Type.Length);
        Array.Copy(ID, 0, msg, 2, ID.Length);
        Array.Copy(PASS, 0, msg, 12, PASS.Length);
        NetworkManager.Instance.sendMsg(msg);
    }

    public void signupUIActive(bool _val)
    {
        if(!_val)
        {
            Instance.StartInputFieldArry[(int)StartInputFieldList.signID].text = "";
            Instance.StartInputFieldArry[(int)StartInputFieldList.signPASS].text = "";

        }
        Instance.StartUIArry[(int)StartUIList.signUI].SetActive(_val);
    }

    public void withDrawalUIActive(bool _val)
    {
        if (!_val)
        {
            Instance.StartInputFieldArry[(int)StartInputFieldList.withDrawaID].text = "";
            Instance.StartInputFieldArry[(int)StartInputFieldList.withDrawalPASS].text = "";
        }
        Instance.StartUIArry[(int)StartUIList.withDrawalUI].SetActive(_val);
    }

    public void resSignUp(byte[] _data)
    {
        // char     Result
        byte Result = _data[7];
        if (Result == (byte)login_Result.Success)
        {
            signupUIActive(false);
        }
        else
            LoginFailUI(true);
    }

    public void resWithDrawal(byte[] _data)
    {
        // char     Result
        byte Result = _data[7];
        if (Result == (byte)login_Result.Success)
        {
            withDrawalUIActive(false);
        }
        else
            LoginFailUI(true);
    }

    public void Login()
    {
        if (Instance.StartInputFieldArry[(int)StartInputFieldList.loginID].text.Length <= 0 || Instance.StartInputFieldArry[(int)StartInputFieldList.loginID].text.Length > 10 || Instance.StartInputFieldArry[(int)StartInputFieldList.loginPASS].text.Length <= 0 || Instance.StartInputFieldArry[(int)StartInputFieldList.loginPASS].text.Length > 15)
        {
            LoginFailUI(true);
            return;
        }
        byte[] Type = BitConverter.GetBytes((short)Protocol.login_Login_req);
        byte[] ID = System.Text.Encoding.ASCII.GetBytes(Instance.StartInputFieldArry[(int)StartInputFieldList.loginID].text);
        byte[] PASS = System.Text.Encoding.ASCII.GetBytes(Instance.StartInputFieldArry[(int)StartInputFieldList.loginPASS].text);

        byte[] msg = new byte[27];
        Array.Copy(Type, 0, msg, 0, Type.Length);
        Array.Copy(ID, 0, msg, 2, ID.Length);
        Array.Copy(PASS, 0, msg, 12, PASS.Length);
        NetworkManager.Instance.sendMsg(msg);
    }

    public void LoginFailUI(bool _flag)
    {
        Instance.StartUIArry[(int)StartUIList.LoginFailUI].SetActive(_flag);
    }

    public void selectChar(int _num)
    {
        if (charArry[_num].Level == 0)
            return;
        Instance.charID = charArry[_num].charNo;
        Instance.charNo = _num;
        spawnChar(charArry[_num].charType, true);
    }

    public void startGame()
    {
        byte[] Type = BitConverter.GetBytes((short)Protocol.login_userSelect_req);
        byte[] charNumber = BitConverter.GetBytes((char)Instance.charNo);
        byte[] objectID = System.Text.Encoding.ASCII.GetBytes(Instance.charID);
        byte[] msg = new byte[28];
        Array.Copy(Type, 0, msg, 0, Type.Length);
        Array.Copy(charNumber, 0, msg, 2, 1);
        Array.Copy(objectID, 0, msg, 3, 25);
        NetworkManager.Instance.sendMsg(msg);
    }

    public InputField charNameField;
    GameObject charTypeName;
    int selectCharType;

    public void createScene()
    {
        if (charArry[2].Level != 0)
        {
            Instance.LobbyUIArry[(int)LobbyUIList.createFailUI].SetActive(true);
            return;
        }

        setSelectorUI(true);

        if (Instance.charTypeName == null)
            Instance.charTypeName = GameObject.Find("charPanel").transform.Find("Text").gameObject;
        Instance.charTypeName.GetComponent<Text>().text = "Ellen";
        Instance.selectCharType = 0;
        spawnChar((int)charType.Ellen);
    }

    public void create_swapBtn()
    {
        if (Instance.charTypeName == null)
            Instance.charTypeName = GameObject.Find("charPanel").transform.Find("Text").gameObject;
        if (Instance.charTypeName.GetComponent<Text>().text == "Ellen")
        {
            Instance.charTypeName.GetComponent<Text>().text = "Babarian";
            spawnChar(1);
            Instance.selectCharType = 1;
        }
        else
        {
            Instance.charTypeName.GetComponent<Text>().text = "Ellen";
            spawnChar(0);
            Instance.selectCharType = 0;
        }
    }

    GameObject deleteNameText;
    bool deleteCheck;
    public void delete_sceneBtn()
    {
        if (Instance.charNo == -1)
        {
            Instance.LobbyUIArry[(int)LobbyUIList.deleteFailUI].SetActive(true);
            return;
        }
        Instance.LobbyUIArry[(int)LobbyUIList.deleteUI].SetActive(true);

        if (Instance.deleteNameText == null)
            Instance.deleteNameText = GameObject.Find("deleteUI").transform.Find("charNameText").gameObject;
        Instance.deleteNameText.GetComponent<Text>().text = charArry[Instance.charNo].Name;
        Instance.deleteCheck = true;

    }

    public void delete_yesBtn()
    {
        if (Instance.deleteCheck == false)
        {
            Instance.LobbyUIArry[(int)LobbyUIList.deleteFailUI].SetActive(true);
            return;
        }
        Instance.proc_deleteChar(Instance.charNo, Instance.charID);
    }

    public void delete_noBtn()
    {
        Instance.LobbyUIArry[(int)LobbyUIList.deleteUI].SetActive(false);
        Instance.deleteCheck = false;
    }

    public void closeNicknamePopup(bool _val)
    {
        if (Instance.LobbyUIArry[(int)LobbyUIList.nameFailUI] == null)
            Instance.LobbyUIArry[(int)LobbyUIList.nameFailUI] = GameObject.Find("nicknameFail").gameObject;
        Instance.LobbyUIArry[(int)LobbyUIList.nameFailUI].SetActive(_val);
    }

    public void create_createBtn()
    {
        if (Instance.charNameField.text.Length == 0 || Instance.charNameField.text.Length >= 10)
        {
            closeNicknamePopup(true);
            return;
        }
        Instance.proc_createChar(Instance.selectCharType, Instance.charNameField.text.ToString());
    }

    public void create_cancleBtn()
    {
        // 네트워크에 캐릭터 리스트 요청 패킷 전송
        Instance.characterListRequest();
        Instance.charNameField.text = "";
        setSelectorUI(false);
    }

    void setSelectorUI(bool _val)
    {
        Destroy(Instance.character);
        Instance.LobbyUIArry[(int)LobbyUIList.createUI].SetActive(_val);
        Instance.LobbyUIArry[(int)LobbyUIList.selectUI].SetActive(!_val);
    }

    void proc_createChar(int _type, string _name)
    {
        // short		Type
        // short		charType
        // WCHAR	nickName[10]

        byte[] msg = new byte[24];

        byte[] Type = BitConverter.GetBytes((short)Protocol.login_createChar_req);
        byte[] charType = BitConverter.GetBytes((short)_type);
        byte[] ID = System.Text.Encoding.Unicode.GetBytes(_name);
        Array.Copy(Type, 0, msg, 0, Type.Length);
        Array.Copy(charType, 0, msg, 2, charType.Length);
        Array.Copy(ID, 0, msg, 4, ID.Length);
        Network.sendMsg(msg);
    }

    void proc_deleteChar(int _index, string _id)
    {
        byte[] msg = new byte[28];

        byte[] Type = BitConverter.GetBytes((short)Protocol.login_deleteChar_req);
        msg[2] = (byte)_index;
        byte[] ID = System.Text.Encoding.ASCII.GetBytes(_id);
        Array.Copy(Type, 0, msg, 0, Type.Length);
        Array.Copy(ID, 0, msg, 3, 25);
        NetworkManager.Instance.sendMsg(msg);
    }

    protected GameObject m_CurrentCheckpoint;
    private GameObject character = null;
    private playerName playerNameTag;

    void setSpawn()
    {
        Instance.m_CurrentCheckpoint = GameObject.Find("Checkpoint");
    }

    void spawnChar(int _type, bool _createMode = false)
    {
        if (Instance.character != null)
            Destroy(Instance.character);

        if (Instance.m_CurrentCheckpoint == null)
            setSpawn();

        Instance.character = Instantiate(Instance.prefabsArry[_type], Instance.m_CurrentCheckpoint.transform.position, Instance.m_CurrentCheckpoint.transform.rotation) as GameObject;
        if (_createMode)
        {
            Instance.playerNameTag = Instance.character.GetComponentInChildren<playerName>();
            Instance.playerNameTag.setPlayer(Instance.character, charArry[Instance.charNo].Name);
        }
        else
        {
            Instance.playerNameTag = Instance.character.GetComponentInChildren<playerName>();
            Instance.playerNameTag.setPlayer(Instance.character, "");
        }
    }

    public void resGameStart(byte[] _data)
    {
        System.UInt64 sessionKey = BitConverter.ToUInt64(_data, 7);
        string Ip = System.Text.Encoding.ASCII.GetString(_data, 15, 16);
        short port = BitConverter.ToInt16(_data, 31);
        Instance.StartCoroutine(LobbyToGame());
        NetworkManager.Instance.setSessionKey(sessionKey);
        NetworkManager.Instance.Net_ConnectoGame(Ip, port);
    }

    public void resLogin(byte[] _data)
    {
        // char     Result

        byte Result = _data[7];
        if (Result == (byte)login_Result.Success)
            toLobby();
        else
            LoginFailUI(true);
    }

    public void toLobby()
    {
        Instance.StartCoroutine(lobbySceneChange());
        Instance.state = nowStatus.Lobby;
    }

    public void toStart()
    {
        Instance.StartCoroutine(startSceneChange());
        Instance.state = nowStatus.Start;
    }

    public IEnumerator startSceneChange()
    {
        yield return Instance.StartCoroutine(Gamekit3D.ScreenFader.FadeSceneOut(Gamekit3D.ScreenFader.FadeType.Loading));
        yield return SceneManager.LoadSceneAsync("Start");
        init_startUI();
        yield return Instance.StartCoroutine(Gamekit3D.ScreenFader.FadeSceneIn());
    }

    public IEnumerator lobbySceneChange()
    {
        yield return Instance.StartCoroutine(Gamekit3D.ScreenFader.FadeSceneOut(Gamekit3D.ScreenFader.FadeType.Loading));
        yield return SceneManager.LoadSceneAsync("Lobby");
        init_Lobby();
        characterListRequest();
        yield return Instance.StartCoroutine(Gamekit3D.ScreenFader.FadeSceneIn());

    }

    IEnumerator LobbyToGame()
    {
        yield return Instance.StartCoroutine(Gamekit3D.ScreenFader.FadeSceneOut(Gamekit3D.ScreenFader.FadeType.Loading));
        yield return SceneManager.LoadSceneAsync("Game");
    }

    public void resCreateChar(byte[] _data)
    {
        // char     Result
        byte Result = _data[7];
        if (Result == (byte)login_Result.Success)
        {
            Instance.setSelectorUI(false);
            Instance.setCharArray(_data);
        }
        else
            Instance.LobbyUIArry[(int)LobbyUIList.createFailUI].SetActive(true);    

    }

    public void resDeleteChar(byte[] _data)
    {
        // char     Result
        byte Result = _data[7];
        if (Result == (byte)login_Result.Success)
        {
            Instance.LobbyUIArry[(int)LobbyUIList.deleteUI].SetActive(false);
            if (Instance.character != null)
                Destroy(Instance.character);
            Instance.setCharArray(_data);
        }
        else
            Instance.LobbyUIArry[(int)LobbyUIList.deleteFailUI].SetActive(true);
    }

    public void setCharArray(byte[] _data)
    {
        // short		Type
        // charArr	Data                  
        // charArr { count count   & charSelect Result[3] } -> charSelect char oid[25], WCHAR nickName[10], int level     
        byte charCount = _data[8];
        byte count;
        byte charNo = 9;
        byte nameNo = 34;
        byte levelNo = 54;
        byte typeNo = 58;
        Debug.Log(_data.ToString());
        if (Slot[0].Name == null)
            setSlot();

        for (count = 0; count < 3; count++)
        {
            if(charCount <= count)
            {
                charArry[count].Name = null;
                charArry[count].Level = 0;
                Slot[count].Name.GetComponent<Text>().text = null;
                Slot[count].Level.GetComponent<Text>().text = null;
                continue;
            }
            charArry[count].charNo = System.Text.Encoding.Default.GetString(_data, charNo + (count * 53), 25);
            charArry[count].Name = System.Text.Encoding.Unicode.GetString(_data, (nameNo + (count * 53)),20);
            charArry[count].Level = BitConverter.ToInt32(_data, (levelNo + (count * 53)));
            charArry[count].charType = BitConverter.ToInt32(_data, typeNo + (count * 53));

            Slot[count].Name.GetComponent<Text>().text = charArry[count].Name;
            Slot[count].Level.GetComponent<Text>().text = charArry[count].Level.ToString();

        }
        Instance.charNo = -1;
        Instance.charID = null;
    }

    void setSlot()
    {
        if (state != nowStatus.Lobby)
            return;
        Slot[0].Name = GameObject.Find("OneBtn").transform.Find("Name").gameObject;
        Slot[0].Level = GameObject.Find("OneBtn").transform.Find("Level").gameObject;
        Slot[1].Name = GameObject.Find("TwoBtn").transform.Find("Name").gameObject;
        Slot[1].Level = GameObject.Find("TwoBtn").transform.Find("Level").gameObject;
        Slot[2].Name = GameObject.Find("ThreeBtn").transform.Find("Name").gameObject;
        Slot[2].Level = GameObject.Find("ThreeBtn").transform.Find("Level").gameObject;
    }

    void characterListRequest()
    {
        byte[] Type = BitConverter.GetBytes((short)Protocol.login_charList_req);
        byte[] msg = new byte[2];
        Array.Copy(Type, 0, msg, 0, Type.Length);
        NetworkManager.Instance.sendMsg(msg);
        return;
    }
}

