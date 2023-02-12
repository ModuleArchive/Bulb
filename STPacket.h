#ifndef STP_MAX
#define STP_MAX 10
#endif

#ifndef STP_SP
#define STP_SP '|'
#endif
#ifndef STP_MD
#define STP_MD '='
#endif
#ifndef STP_ING
#define STP_ING '!'
#endif
#ifndef STP_OPTIMISE
#define STP_OPTIMISE true
#endif

#ifndef STPacket_h
#define STPacket_h 1

String ingReplace(String data, char ing = STP_ING){
    String str = "";

    for (int i = 0; i < data.length() - 1; i++)
    {
        if((data[i] != ing) || (i > 0 && data[i] == ing && data[i - 1] == ing)){
            str[i] = data[i];
        }
    }
    
    return str;
}

String getValue(String data, char separator, int index, char ing = STP_ING)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if ((data.charAt(i) == separator || i == maxIndex) && ( i == 0 ? true : (data.charAt(i - 1) == ing ? data.charAt(i - 2) == ing : true)))
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

int countOf(String str, char sp, char ing = STP_ING)
{
    int count = 0;
    for (int i = 0; i < str.length(); i++)
    {
        if (str[i] == sp && ( i == 0 ? true : (str.charAt(i - 1) == ing ? str.charAt(i - 2) == ing : true)))
            count++;
    }
    return count;
}

class STPacket
{
public:
    STPacket(String str)
    {
        from(str);
    }

    STPacket()
    {
    }

    void from(String str)
    {
        int count = countOf(str, STP_SP, STP_ING);

        if (count > STP_MAX)
        {
            count = STP_MAX;
        }

        __length = count;

        if (count > 0)
        {
            Serial.println(count);
            for (int i = 0; i <= count; i += 1)
            {
                String strw = getValue(str, STP_SP, i, STP_ING);
                if(countOf(strw, STP_MD, STP_ING) > 0){
                    String key = getValue(strw, STP_MD, 0, STP_ING);
                    String value = strw.substring(key.length() + 1);
                    key.replace(STP_ING + STP_SP, STP_SP);
                    key.replace(STP_ING + STP_MD, STP_MD);
                    value.replace(STP_ING + STP_SP, STP_SP);
                    value.replace(STP_ING + STP_MD, STP_MD);
                    ingReplace(key, STP_ING);
                    ingReplace(value, STP_ING);
                    __str[i][0] = key != "" ? key : String(i);
                    __str[i][1] = value;
                }else{
                    __str[i][0] = String(i);
                    strw.replace(STP_ING + STP_SP, STP_SP);
                    strw.replace(STP_ING + STP_MD, STP_MD);
                    ingReplace(strw, STP_ING);
                    __str[i][1] = strw;
                }
            }
        }
    }

    String get(int pos)
    {
        if (pos >= 0 && pos <= __length)
        {
            return __str[pos][1];
        }
        return "";
    }

    String key(int pos)
    {
        if (pos >= 0 && pos <= __length)
        {
            return __str[pos][0];
        }
        return "";
    }

    String get(String key)
    {
        if (key != "" && key != NULL)
        {
            for (int i = 0; i <= __length; i += 1)
            {
                if (__str[i][0] == key)
                {
                    return __str[i][0];
                }
            }
        }
        return "";
    }

    int indexOf(String val)
    {
        if (val != "" && val != NULL)
        {
            for (int i = 0; i <= __length; i += 1)
            {
                if (__str[i][1] == val)
                {
                    return i;
                }
            }
        }
        return -1;
    }

    int indexOfKey(String key)
    {
        if (key != "" && key != NULL)
        {
            for (int i = 0; i <= __length; i += 1)
            {
                if (__str[i][0] == key)
                {
                    return i;
                }
            }
        }
        return -1;
    }

    String operator[](int index)
    {
        return get(index);
    }

    String operator[](String index)
    {
        return get(index);
    }

    int length()
    {
        return __length + 1;
    }

    void set(String str)
    {
        __length += 1;
        __str[__length][0] = String(__length);
        __str[__length][1] = String(str);
    }

    void set(String key, String str)
    {
        int getw = indexOf(key);
        if (getw == -1 && key != "")
        {
            __length += 1;
            __str[__length][0] = String(key);
            __str[__length][1] = String(str);
        }
        else
        {
            if (key == "")
            {
                __str[getw][0] = String(getw);
                __str[getw][1] = String(str);
            }
            else
            {
                __str[getw][0] = String(key);
                __str[getw][1] = String(str);
            }
        }
    }

    void setKey(int pos, String key)
    {
        if(pos <= __length){
            __str[pos][0] = key;
        }
    }

    void pop()
    {
        if (__length > 0)
        {
            __str[__length][0] = "";
            __str[__length][1] = "";
            __length -= 1;
        }
    }

    String toString()
    {
        String pkt = "";
        for (int i = 0; i <= __length; i++)
        {
            if (__str[i][0] != "" && __str[i][1] != "")
            {
                String key = __str[i][0];
                String val = __str[i][1];
                key.replace(STP_SP, STP_ING + STP_SP);
                key.replace(STP_MD, STP_ING + STP_MD);
                key.replace(STP_ING, STP_ING + STP_ING);
                val.replace(STP_SP, STP_ING + STP_SP);
                val.replace(STP_MD, STP_ING + STP_MD);
                val.replace(STP_ING, STP_ING + STP_ING);
                if(STP_OPTIMISE && key != String(i)){
                    pkt += key;
                    pkt += STP_MD;
                }
                pkt += val;

                if (i < __length)
                {
                    pkt += String(STP_SP);
                }
            }
        }

        return pkt;
    }

    void dump()
    {
        Serial.print("Length Of Packet: ");
        Serial.println(__length + 1);
        for (int i = 0; i <= __length; i++)
        {
            if (__str[i][0] != "" && __str[i][1] != "")
            {
                Serial.print(i);
                Serial.print("# ");
                Serial.print(__str[i][0]);
                Serial.print(" -> ");
                Serial.print(__str[i][1]);
                Serial.print("; ");

                if (i < __length)
                {
                    Serial.println("AND.");
                }
                else
                {
                    Serial.println("END.");
                }
            }
            else
            {
                Serial.print(i);
                Serial.print("# ");
                Serial.print(__str[i][0]);
                Serial.print(" => ");
                Serial.print(__str[i][1]);
                Serial.print("; ");
            }
        }
    }

private:
    int __length = 0;
    String __str[STP_MAX][2] = {};
};



#endif