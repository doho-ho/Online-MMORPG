using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Text;

public struct dataObject {
    public byte[] data;
    public System.DateTime time;
};

public class StateObject
{
    public Socket workSocket = null;
    public const int bufferSize = 512;
    public byte[] Buffer = new byte[bufferSize];
}

public class NetworkManager : MonoBehaviour {

    protected static NetworkManager s_Instance;

    public static AuthManager Auth;
    public static GameManager Game;
    public static Queue<dataObject> packetQueue = new Queue<dataObject>();

    private static Socket client;
    private static StateObject state = new StateObject();
    private static bool connectFlag=false;
    private static bool reconnectedFlag = false;

    public int packetProcessPerSec = 100;

    private static byte Code = 0x77;
    private static byte KeyCode = 0xb6;

    protected string IP;
    protected int Port;
    protected System.UInt64 sessionKey;

    public enum nowStatus
    {
        Start =0, Lobby, Game,
    }

    public static NetworkManager Instance
    {
        get
        {
            if (s_Instance != null)
                return s_Instance;

            s_Instance = FindObjectOfType<NetworkManager>();

            if (s_Instance != null)
                return s_Instance;

            NetworkManager NetMng = Resources.Load<NetworkManager>("NetworkManager");
            s_Instance = Instantiate(NetMng);

            return s_Instance;
        }
    }

    void Awake()
    {
        Auth = AuthManager.Instance;
        if (Instance != this)
        {
            Destroy(gameObject);
            return;
        }

        DontDestroyOnLoad(gameObject);
    }
    // Use this for initialization
    void Start () {
        setAddress();
        IP = "127.0.0.1";
        Port = 15000;
        StartClient();
	}
	
	// Update is called once per frame
	void Update () {
        if (reconnectedFlag)
            StartClient();
        int Count = 0;
        int maxPacketProcessCount = Instance.packetProcessPerSec;
        for (Count=0; Count < maxPacketProcessCount; Count++)
        {
            float nowTime = Time.time;
            if (packetQueue.Count == 0)
                break; 
            dataObject data = dequeue();
            completeRecv(data.data);
        }
	}

    // ManualResetEvent instances signal completion.  
    private static ManualResetEvent connectDone = new ManualResetEvent(false);
    private static ManualResetEvent sendDone = new ManualResetEvent(false);
    private static ManualResetEvent receiveDone = new ManualResetEvent(false);

    private void setAddress()
    {

    }

    private static void StartClient()
    {
        if (connectFlag)
            return;

        // Connect to a remote device.  
        try
        {
            // Establish the remote endpoint for the socket.  
            // The name of the   
            // remote device is "host.contoso.com".  
            if (Instance.Port == 0)
                throw Exception e;
            IPAddress ipAddress = IPAddress.Parse(Instance.IP);
            IPEndPoint remoteEP = new IPEndPoint(ipAddress, Instance.Port);

            // Create a TCP/IP socket.  
            Socket client = new Socket(ipAddress.AddressFamily,
                SocketType.Stream, ProtocolType.Tcp);

            // Connect to the remote endpoint.  
            client.BeginConnect(remoteEP,
                new AsyncCallback(ConnectCallback), client);

            // Write the response to the console.  
            Debug.Log("Receive start!");

        }
        catch (Exception e)
        {
            Console.WriteLine(e.ToString());
        }
    }

    private static void ConnectCallback(IAsyncResult ar)
    {
        try
        {
            // Retrieve the socket from the state object.  
            client = (Socket)ar.AsyncState;

            // Complete the connection.  
            client.EndConnect(ar);

            connectFlag = true;
            reconnectedFlag = false;
            // 작업처리
            Console.WriteLine("Socket connected to {0}",
                client.RemoteEndPoint.ToString());

            // Signal that the connection has been made.  
            connectDone.Set();

            // Receive the response from the remote device.  
            Recv();
        }
        catch (Exception e)
        {
            Console.WriteLine(e.ToString());
        }
    }

    private static void Recv()
    {
        try
        {
            state.workSocket = client;
            // Begin receiving the data from the remote device.  
            client.BeginReceive(state.Buffer, 0, StateObject.bufferSize, SocketFlags.None,
                new AsyncCallback(RecvCallback), state);
        }
        catch (Exception e)
        {
            Debug.Log(e.ToString());
        }
    }

    private static void RecvCallback(IAsyncResult ar)
    {
        try
        {
            // Retrieve the state object and the client socket   
            // from the asynchronous state object.  
            StateObject state = (StateObject)ar.AsyncState;
            
            // Read data from the remote device.  
            int bytesRead = client.EndReceive(ar);

            if (bytesRead > 0)
            {
                separatePacket(state.Buffer,bytesRead);
                
                // Get the rest of the data.  
                client.BeginReceive(state.Buffer, 0, StateObject.bufferSize, SocketFlags.None,
                    new AsyncCallback(RecvCallback), state);
            }
            else
            {
                // Signal that all bytes have been received.  
                receiveDone.Set();
            }
        }
        catch (Exception e)
        {
            connectFlag = false;
        //    reconnectedFlag = true;
            Console.WriteLine(e.ToString());
        }
    }

    private void Send(byte[] data)
    {
        // Convert the string data to byte data using ASCII encoding.  
        //byte[] byteData = Encoding.UTF8.GetBytes(data);

        // Begin sending the data to the remote device.  
        client.BeginSend(data, 0, data.Length, 0,
            new AsyncCallback(SendCallback), client);
    }

    private void SendCallback(IAsyncResult ar)
    {
        try
        {
            // Retrieve the socket from the state object.  
            Socket client = (Socket)ar.AsyncState;

            // Complete sending the data to the remote device.  
            int bytesSent = client.EndSend(ar);
            Console.WriteLine("Sent {0} bytes to server.", bytesSent);

            // Signal that all bytes have been sent.  
            sendDone.Set();
        }
        catch (Exception e)
        {
            Console.WriteLine(e.ToString());
        }
    }

    public void sendMsg(byte[] _msg)
    {
        byte[] data = Encode(_msg);
        Send(data);
    }


    private static byte[] Encode(byte[] msg)
    {

        short len = (short)msg.Length;
        byte[] data = new byte[5 + len];

        byte randCode = (byte)(UnityEngine.Random.Range(0, 32767) % 256);
        byte checkSum = 0;
        int count;
        for (count = 0; count < msg.Length; count++)
        {
            checkSum += msg[count];
            data[(count + 5)] = (byte)(msg[count] ^ randCode);
        }
      
        checkSum = (byte)((checkSum%256) ^ randCode);

        data[0] = Code;
        data[1] = (byte)len;
        data[2] = (byte)(len >> 8);
        data[3] = (byte)(randCode ^ KeyCode);
        data[4] = checkSum;

        return data;
    }

    private static void separatePacket(byte[] _data, int _len)
    {
        int dataPos;
        for(dataPos = 0; dataPos != _len;)
        {
            int Len = BitConverter.ToInt16(_data, (dataPos + 1)) + 5;
            byte[] data = new byte[Len];
            Array.Copy(_data, dataPos, data, 0, Len);
            if (Decode(data))
                enqueue(data);
            dataPos += Len;
        }
    }
    
    private static bool Decode(byte[] _data)
    {
        byte checkSum = 0;
        int len = BitConverter.ToInt16(_data, 1)+5;
        _data[3] ^= KeyCode;
        _data[4] ^= _data[3];
        int count;
        for (count = 5; count < len; count++)
        {
            _data[count] ^= _data[3];
            checkSum += _data[count];
        }

        checkSum = (byte)(checkSum % 256);
         

        if (_data[0] != Code)
            return false;
        if (_data[4] != checkSum)
            return false;

        return true;
    }

    private static void completeRecv(byte[] _data)
    {
        short Type = BitConverter.ToInt16(_data, 5);

        switch (Type)
        {
            case (short)Protocol.login_signUp_res:
                AuthManager.Instance.resSignUp(_data);
                break;
            case (short)Protocol.login_withDrawal_res:
                AuthManager.Instance.resWithDrawal(_data);
                break;
            case (short)Protocol.login_Login_res:
                AuthManager.Instance.resLogin(_data);
                break;
            case (short)Protocol.login_charList_res:
                AuthManager.Instance.setCharArray(_data);
                break;
            case (short)Protocol.login_deleteChar_res:
                AuthManager.Instance.resDeleteChar(_data);
                break;
            case (short)Protocol.login_createChar_res:
                AuthManager.Instance.resCreateChar(_data);
                break;
            case (short)Protocol.login_userGameConnect_res:
                AuthManager.Instance.resGameStart(_data);
                break;
            case (short)Protocol.game_newUser_res:
                GameManager.Instance.Game_createChar(_data,false);
                break;
            case (short)Protocol.game_newChar_res:
                GameManager.Instance.Game_createChar(_data,true);
                break;
            case (short)Protocol.game_deleteUser_res:
                GameManager.Instance.Game_deleteChar(_data);
                break;
            case (short)Protocol.game_move_res:
                GameManager.Instance.Game_moveRes(_data);
                break;
            case (short)Protocol.game_stop_res:
                GameManager.Instance.Game_stopRes(_data);
                break;
            case (short)Protocol.game_sync_res:
                GameManager.Instance.Game_syncRes(_data);
                break;
            case (short)Protocol.game_attack_res:
                GameManager.Instance.Game_attackRes(_data);
                break;
            case (short)Protocol.game_hit_res:
                GameManager.Instance.Game_hitRes(_data);
                break;
            case (short)Protocol.game_Recovery_res:
                GameManager.Instance.Game_autoRecovery(_data);
                break;
            default:
                break;
        }
    }

    public static void enqueue(byte[] _data)
    {
        dataObject node = new dataObject();
        node.data = _data;
        node.time = System.DateTime.Now;
        
        packetQueue.Enqueue(node);
    }

    public dataObject dequeue()
    {
        return packetQueue.Dequeue();
    }

    public void Net_ConnectoGame(string _ip, short _port)
    {
        Disconnect();
        IP = _ip;
        Port = _port;
        StartClient();
    }

    public void Disconnect()
    {
        // Release the socket.  
        client.Shutdown(SocketShutdown.Both);
        client.Close();
        connectFlag = false;
    }

    public void setSessionKey(System.UInt64 _sessionKey)
    {
        Instance.sessionKey = _sessionKey;
    }

    public void GAME_LOGIN()
    {
        byte[] Type = BitConverter.GetBytes((short)Protocol.game_loginUser_req);
        byte[] sessionKey = BitConverter.GetBytes(Instance.sessionKey);
        byte[] version = BitConverter.GetBytes((int)0.1);
        byte[] msg = new byte[14];
        Array.Copy(Type, 0, msg, 0, Type.Length);
        Array.Copy(sessionKey, 0, msg, 2, sessionKey.Length);
        Array.Copy(version, 0, msg, 10, version.Length);
        Instance.sendMsg(msg);
    }

}
