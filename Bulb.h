#include <Arduino.h>
#include <Bulb/STPacket.h>

#ifndef BULB_MAX_TIMEOUT
#define BULB_MAX_TIMEOUT 2000
#endif
#ifndef BULB_DEBUG
#define BULB_DEBUG false
#endif

#ifndef BULB_AUTO_AUTH
#define BULB_AUTO_AUTH true
#endif

#ifndef BULB_MAX_JSON_DATA_CAPACITY
#define BULB_MAX_JSON_DATA_CAPACITY 1000
#endif

#ifndef BULB_INTERFACE_NAME
#define BULB_INTERFACE_NAME "bulb.generic"
#endif

#ifndef BULB_INTERFACE_VERSION
#define BULB_INTERFACE_VERSION 1
#endif

#ifndef BULBLIB
#define BULBLIB 1

enum BULB_STATE
{
    GG_NOTCONNECTED,
    GG_REFUSED,
    GG_TIMEOUT,
    GG_CONSOLE_TIMEOUT,
    GG_OPEN_TIMEOUT,
    GG_OLDVERSION,
    GG_NEWVERSION,
    GG_CONNECTED,
    // SENT STATE
    GGS_SUCCESS,
    GGS_UNCONNECTED,
    GGS_BUSY,
    // REASONS
    GGR_APP_ERROR,
    GGR_ALREADY_DISCONNECTED,
    GGR_USER_INTERRUPT,
    GGR_CUSTOM,
    GGR_INTERRUPT,
    GGR_BUSY,
    // LISTENERS
    GGC_DISCONNECT,
    GGC_CONNECT,
    GGC_AUTH,
};

/*
class BulbStream {
    public:
        BulbStream(*gg) {
            __id = String(random(0, 1000));
            __gg = gg;
        };

        void send(){

        }
    private:
        String __id;
        BulbInterface *__gg;
}; */

class BulbInterface
{
public:
    BulbInterface()
    {
        if (BULB_DEBUG == true)
        {
            state = GG_CONNECTED;
            token = "debug";
        }
    }

    int connect(String tokenr, bool broadcast = false)
    {
        if (__isBusy)
            return -1;


        __isBusy = true;
        int defreturn;
            //Serial.println(token);

        // Trying To Know If Bulb APP is Responding
        if (tokenr != "")
        {
            defreturn = GG_CONNECTED;
            token = tokenr;
            state = GG_CONNECTED;
            if (broadcast)
            {
                connectivityListener(GGC_CONNECT, defreturn, 0);
            }
        }

        __isBusy = false;

        return defreturn;
    }

    void main(String read)
    {
        read.replace("\r", "");
        read.replace("\n", "");

        if (__isReading || __isBusy)
        {
            return;
        }

        __isReading = true;

        if (read.startsWith("//*.connect:"))
        {
            read = read.substring(12);
            // connectivityListener(GGC_AUTH, -1, 0);
            /* if (BULB_AUTO_AUTH)
            { */
            connect(read, true);
            //}
        }
        else if (read == "//*.lib")
        {
            SendPrefixedData("lib:" + String(BULBLIB));
        }
        else if (read == "//*.i")
        {
            SendPrefixedData("i:" + String(BULBLIB) + "|" + String(BULB_INTERFACE_NAME) + "|" + String(BULB_INTERFACE_VERSION) + "|" + (token != "" ? token : "*"));
        }
        else if (read == "//*.at")
        {
            SendPrefixedData("at");
        }
        else if (read == "//*.token")
        {
            SendPrefixedData("token:" + (token != "" ? token : "*"));
        }
        else if (read == "//*.name")
        {
            SendPrefixedData(String("name:") + String(BULB_INTERFACE_NAME));
        }
        else if (read == "//*.version")
        {
            SendPrefixedData(String("version:") + String(BULB_INTERFACE_VERSION));
        }
        else
        {
            if (state == GG_CONNECTED)
            {

                if (read.startsWith("//*.s:"))
                {
                    read = read.substring(6);

                    String key;
                    String args;
                    STPacket packet;

                    if (read.indexOf("$") != -1 ? getValue(read, '$', 0).length() > 0 : true)
                    {
                        if (read.indexOf("$") != -1)
                        {
                            key = getValue(read, '$', 0);
                            args = getValue(read, '$', 1);
                        }
                        else
                        {
                            key = read;
                            args = "";
                        }
                    }
                    else
                    {
                        key = "main";
                        args = read;
                    }
                    if (args != "")
                    {
                        packet.from(args);
                    }
                    key.replace("$", "");
                    eventListener(key, packet);
                }
                else if (read == "//*.disconnect:apperror")
                {
                    state = GG_NOTCONNECTED;
                    connectivityListener(GGC_DISCONNECT, GGR_APP_ERROR, 0);
                    token = "";
                }
                else if (read == "//*.disconnect:userinterrupt")
                {
                    state = GG_NOTCONNECTED;
                    connectivityListener(GGC_DISCONNECT, GGR_USER_INTERRUPT, 0);
                    token = "";
                }
                else if (read == "//*.disconnect:custom:")
                {
                    state = GG_NOTCONNECTED;
                    connectivityListener(GGC_DISCONNECT, GGR_CUSTOM, read.substring(22).toInt());
                    token = "";
                }
                else if (read == "//*.disconnect:interrupt")
                {
                    state = GG_NOTCONNECTED;
                    connectivityListener(GGC_DISCONNECT, GGR_INTERRUPT, 0);
                    token = "";
                }
            }
            else
            {
                if (read.startsWith("//*."))
                {
                    SendPrefixedData("notConnected");
                }
            }
        }

        __isReading = false;
    }

    /**
     * @brief Sends an event with arguments of strings
     * @param cmd String
     * @param val String[]
     */
    int emit(String cmd, STPacket packet)
    {
        if (isBusy())
        {
            return GGS_BUSY;
        }

        String arg = packet.toString();

        if (state != GG_CONNECTED)
            return GGS_UNCONNECTED;

        SendPrefixedAuthedData("s:" + cmd + "$" + arg);

        return GGS_SUCCESS;
    }

    /**
     * @brief Sends an event with arguments of single string
     * @param cmd String
     * @param val String
     */
    int emit(String cmd, String arg)
    {
        if (isBusy())
        {
            return GGS_BUSY;
        }

        SendPrefixedAuthedData("s:" + cmd + "$" + arg);

        return GGS_SUCCESS;
    }

    /**
     * @brief Sends an event with arguments of key and value
     * @param cmd String
     * @param val String
     */
    int emit(String cmd, String key, String val)
    {
        if (isBusy())
        {
            return GGS_BUSY;
        }

        SendPrefixedAuthedData("s:" + cmd + "$" + key + STP_MD + val);

        return GGS_SUCCESS;
    }

    /**
     * @brief Sends an event
     * @param cmd String
     */
    int emit(String cmd)
    {
        if (isBusy())
        {
            return GGS_BUSY;
        }

        SendPrefixedAuthedData("s:" + cmd);

        return GGS_SUCCESS;
    }
    /*  int send(int num){
         SendPrefixedAuthedData("integerdata:" + String(num));
     }

     int send(String key, String val){
         SendPrefixedAuthedData("strdata:" + key + ":" + val);
     }

     int send(String key, int val){
         SendPrefixedAuthedData("strdata:" + key + ":" + String(val));
     }

     int send(int key, String val){
         SendPrefixedAuthedData("intdata:" + String(key) + ":" + val);
     }

     int send(int key, int val){
         SendPrefixedAuthedData("intdata:" + String(key) + ":" + String(val));
     } */

    void (*eventListener)(String cmd, STPacket packet);
    void (*connectivityListener)(int mode, int result, int arg);

    int getState()
    {
        return state;
    };

    bool isBusy()
    {
        return __isBusy;
    };

    bool isReading()
    {
        return __isReading;
    };

    int disconnect(int reason, int val)
    {
        if (state != GG_CONNECTED)
        {
            return GGR_ALREADY_DISCONNECTED;
        }
        __isBusy = true;
        String rs = "";
        switch (reason)
        {
        case GGR_INTERRUPT:
            rs = "interrupt";
            break;
        case GGR_USER_INTERRUPT:
            rs = "userinterrupt";
            break;
        case GGR_APP_ERROR:
            rs = "apperror";
            break;
        default:
            rs = "custom$" + String(val);
        }
        SendPrefixedData("disconnect." + rs);
        __isBusy = false;
    };

    String getToken()
    {
        return token;
    };

private:
    void SendPrefixedData(String data)
    {
        Serial.println("//*." + data);
    }
    void SendPrefixedAuthedData(String data)
    {
        Serial.println("//" + token + "." + data);
    }

    int state = GG_NOTCONNECTED;
    bool __isBusy = false;
    bool __isReading = false;
    String token = "";
};

#endif

#ifndef BULB_MULTI_INSTANCE
/// @brief Create Bulb Instance
BulbInterface Bulb;
#define BULB_MULTI_INSTANCE
#endif