#define MOTOR_1_PWM 5
#define MOTOR_1_DIR 7
#define MOTOR_2_PWM 6
#define MOTOR_2_DIR 8

#define NO_REC_VALS 2 // Ture number (1-Based index)

float pos_x = 0;
float pos_y = 0;

bool read_serial_data()
{
    int i = 0;
    char rec = '0';
    String out = "";
    String recArray[NO_REC_VALS - 1];
    bool valid = false;

    while (Serial.available() > 0)
    {
        rec = Serial.read();
        //  Serial.println("anything received");
        // Serial.println(rec);

        if (rec == '*' && !valid)
        {
            //      Serial.println(rec);
            valid = false;
        }
        if (rec == '*' && valid)
        {
            valid = false;
            pos_x = recArray[0].toFloat();
            pos_y = recArray[1].toFloat();

            return true;
        }
        if (valid && rec != ',')
        {
            //      Serial.print(rec);
            out += rec;
            // Serial.print(out);
        }
        else if (valid && rec == ',')
        {
            recArray[i] = out;
            out = "";
            i++;
        }
        if (!valid && rec == '$')
        {
            valid = true;
            out = "";
            i = 0;
        }
    }

    return false;
}

void setup()
{

    pinMode(MOTOR_1_PWM, OUTPUT);
    pinMode(MOTOR_1_DIR, OUTPUT);
    pinMode(MOTOR_2_PWM, OUTPUT);
    pinMode(MOTOR_2_DIR, OUTPUT);

    // Serial.begin(115200);
    Serial.begin(9600);
}

void loop()
{
    bool ok = read_serial_data();

    // Serial.print(ok ? "OK" : "Not OK");
    // Serial.print("\t X Postion ==> ");
    // Serial.print(pos_x);
    // Serial.print("\t Y Postion ==> ");
    // Serial.println(pos_y);

    if (abs(pos_x) > 10)
    {
        if (abs(pos_x) < 30)
            analogWrite(MOTOR_2_PWM, 70);
        else
            analogWrite(MOTOR_2_PWM, map(abs(pos_x), 30, 100, 70, 120));

        if (pos_x < 0)
            digitalWrite(MOTOR_2_DIR, HIGH);
        else
            digitalWrite(MOTOR_2_DIR, LOW);
    }
    else
    {
        analogWrite(MOTOR_2_PWM, 0);
    }

    if (abs(pos_y) > 10)
    {
        if (abs(pos_x) < 30)
            analogWrite(MOTOR_1_PWM, 70);
        else
        analogWrite(MOTOR_1_PWM, map(abs(pos_y), 30, 100, 70, 120));
        if (pos_y > 0)
            digitalWrite(MOTOR_1_DIR, HIGH);
        else
            digitalWrite(MOTOR_1_DIR, LOW);
    }
    else
    {
        analogWrite(MOTOR_1_PWM, 0);
    }

    delay(250);
}