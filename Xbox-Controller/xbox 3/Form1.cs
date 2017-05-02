using System;
using System.IO;
using System.Net;
using System.Text;
using System.Net.Sockets;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using SharpDX;
using SharpDX.XInput;
using NativeWifi;
using System.Net;
using System.Net.NetworkInformation;
using System.Collections.ObjectModel;
using System.Threading;

namespace xbox_3
{
    
    public partial class Form1 : Form
    {
        
        Controller controller;
        Gamepad gamepad;
        double LeftTrigger;
        private State stateOld;
        public bool connected = false;
        private double normalizedLX, normalizedLY;
        private string temp = "000";
        private string initial = "76464";  //inital static packet
        private string str = "76464"; 
        private static WlanClient wlan = new WlanClient();




        public Form1()
        {
            InitializeComponent();
            timer3.Enabled = true;
            textBox3.Text = "Press Start to start the lawnmower  \r\nPress Right button to stop the lawnmower  \r\nPress X to turn on blade \r\nPress Y to turn off Blade \r\nPress A to increase speed \r\nPress B to decrease speed";
            textBox1.Text = "Controller Settings:";
        }

        public void Wifi()   //check if connected to lawnmower if not stop mower
        {

           
            Collection<String> connectedSsids = new Collection<string>();
            string network = "LAWNMOWER";
            bool Connection = NetworkInterface.GetIsNetworkAvailable();
            try
            {
                if (Connection == true)
                {

                    foreach (WlanClient.WlanInterface wlanInterface in wlan.Interfaces)
                    {

                        Wlan.Dot11Ssid ssid = wlanInterface.CurrentConnection.wlanAssociationAttributes.dot11Ssid;
                        connectedSsids.Add("garbage");
                        connectedSsids.Add(new String(Encoding.ASCII.GetChars(ssid.SSID, 0, (int)ssid.SSIDLength)));
                        if (!((connectedSsids.Contains("garbage")) && (connectedSsids.Contains(network))))
                        {
                            char[] arr = str.ToCharArray();
                            arr[0] = '0';
                            arr[1] = '0';
                            arr[2] = '0';
                            arr[3] = '0';
                            arr[4] = '0';
                            str = new string(arr);
                            SendPacket(str);
                            textBox4.Text = "Lost Connection";
                            textBox2.Text = str;
                        }
                        else
                            textBox4.Text = "Connected";

                    }
                }
            }
            catch
            {
                char[] arr = str.ToCharArray();
                arr[0] = '0';
                arr[1] = '0';
                arr[2] = '0';
                arr[3] = '0';
                arr[4] = '0';
                str = new string(arr);
                SendPacket(str);
               textBox4.Text="Lost connection";
                    
                
                textBox2.Text = str;
            }
        }
            



        public void SendPacket(string x)
        {
            Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Dgram,ProtocolType.Udp);
            IPAddress serverAddr = IPAddress.Parse("192.168.4.1");
            IPEndPoint endPoint = new IPEndPoint(serverAddr, 4210);
            string head = "&";
            string tail = "@";
            x = head + x + tail; ;
            byte[] packet = Encoding.ASCII.GetBytes(x);
            sock.SendTo(packet, endPoint);
            
        }
        public void Connect() //conect controller
        {
            controller = new Controller(UserIndex.One);
            connected = controller.IsConnected;
            if (connected)
                MessageBox.Show("Controller has successfully connected. Click Ok to continue");
            if (!connected)
            {
                MessageBox.Show("ERROR: Controller did not conncect.");
                
            }
        } 
        void Joystick()
        {
            State stick = controller.GetState();
            // Joystick controlls
            double x = (stick.Gamepad.LeftThumbX);
            double y = (stick.Gamepad.LeftThumbY);

           double magnitude = Math.Sqrt(x * x + y * y);
            //determine the direction the controller is pushed
            normalizedLX =( x / magnitude)*100;
            normalizedLY = (y / magnitude)*100;
            string xval = Convert.ToString(normalizedLX);
            string yval = Convert.ToString(normalizedLY);
            MessageBox.Show(xval+ "," + yval);
        }
        void GetInput()
        {
            State j = controller.GetState();
            LeftTrigger=j.Gamepad.LeftTrigger;
            
            State stateNew = controller.GetState();
            //MessageBox.Show(trig);
            
            //buttons controlls
           
            
            if (this.stateOld.Gamepad.Buttons == GamepadButtonFlags.RightShoulder && stateNew.Gamepad.Buttons == GamepadButtonFlags.RightShoulder)   //stop button
            {
                char[] arr = str.ToCharArray();
                char[] array = temp.ToCharArray();
                array[0] = '0';
                array[1] = '0';
                array[2] = '0';
                arr[1] = '0';
                arr[2] = '0';
                arr[3] = '0';
                arr[4] = '0';
                str = new string(arr);
                temp = new string(array);
                String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
                str = str.Remove(0, 1).Insert(0, binary);
                SendPacket(str);
                textBox2.Text = str;

            }
            
            stateOld = stateNew;
         }
       
       /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        private void Navigate()
        {
            State stateNew = controller.GetState();
            if (LeftTrigger > 0)
            {
                if (this.stateOld.Gamepad.Buttons == GamepadButtonFlags.A && stateNew.Gamepad.Buttons == GamepadButtonFlags.A)  //increase speed
                {
                    // MessageBox.Show("a pressed");
                    char[] arr = str.ToCharArray();
                    string speed;
                    string bit_1 = (arr[1]).ToString();
                    string bit_2 = (arr[2]).ToString();
                    string val = bit_1 + bit_2;    //2 bit hex value to be added
                                                   // MessageBox.Show(val);
                    if (val == "F0")   //F0 = 240 last value without going over 255
                    {
                        speed = "FF";
                        textBox5.Text = "Max Speed";
                    }
                    else
                    {
                        int intFromHex = int.Parse(val, System.Globalization.NumberStyles.HexNumber) + 20;  //increment hex value by 20
                        speed = intFromHex.ToString("X");
                    }
                    arr[1] = speed[0];
                    arr[2] = speed[1];
                    arr[3] = speed[0];
                    arr[4] = speed[1];
                    str = new string(arr);
                    SendPacket(str);
                    textBox2.Text = str;


                }

                if (this.stateOld.Gamepad.Buttons == GamepadButtonFlags.B && stateNew.Gamepad.Buttons == GamepadButtonFlags.B)  //decrease speed
                {
                    char[] arr = str.ToCharArray();
                    string speed;
                    string bit_1 = (arr[1]).ToString();
                    string bit_2 = (arr[2]).ToString();
                    string val = bit_1 + bit_2;    //2 bit hex value to be added
                    string dec = Convert.ToString(Convert.ToInt32(val, 16), 10);
                    if (Convert.ToInt32(dec) < 36)
                    {
                        speed = "00";
                        textBox5.Text = "0";
                    }


                    else
                    {
                        int intFromHex = int.Parse(val, System.Globalization.NumberStyles.HexNumber) - 20;  //decrement hex value by 20
                        speed = intFromHex.ToString("X");
                    }
                    arr[1] = speed[0];
                    arr[2] = speed[1];
                    arr[3] = speed[0];
                    arr[4] = speed[1];
                    str = new string(arr);
                    SendPacket(str);
                    textBox2.Text = str;
                }

                if (this.stateOld.Gamepad.Buttons == GamepadButtonFlags.Start && stateNew.Gamepad.Buttons == GamepadButtonFlags.Start)
                {
                    char[] decoy = initial.ToCharArray();
                    char[] arr = temp.ToCharArray();
                    char[] packet = str.ToCharArray();
                    arr[0] = '1';
                    arr[1] = '1';   //assuming 0 is backwards
                    arr[2] = '1';
                    temp = new string(arr);

                    packet[1] = decoy[1];
                    packet[2] = decoy[2];
                    packet[3] = decoy[3];
                    packet[4] = decoy[4];
                    str = new string(packet);
                    String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
                    str = str.Remove(0, 1).Insert(0, binary);
                    SendPacket(str);
                    textBox2.Text = str;
                }
              
                if (this.stateOld.Gamepad.Buttons == GamepadButtonFlags.X && stateNew.Gamepad.Buttons == GamepadButtonFlags.X)  //turn on blade
                {
                    if (temp[0] != '1')
                    {
                       
                        char[] speed = str.ToCharArray();
                        /*speed[1] = '0';
                        speed[2] = '0';
                        speed[3] = '0';
                        speed[4] = '0';*/
                        temp = temp.Remove(0, 1).Insert(0, "1");  //100
                       
                        str = new string(speed);
                        String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
                        str = str.Remove(0, 1).Insert(0, binary);
                        SendPacket(str);
                        textBox2.Text = str;
                        button3.BackColor = SystemColors.Control;
                        blade_on.BackColor = System.Drawing.Color.LightSkyBlue;
                    }

                }
                if (this.stateOld.Gamepad.Buttons == GamepadButtonFlags.Y && stateNew.Gamepad.Buttons == GamepadButtonFlags.Y)  //turn off blade
                {
                   
                        char[] speed = str.ToCharArray();
                        speed[1] = '0';
                        speed[2] = '0';
                        speed[3] = '0';
                        speed[4] = '0';
                        temp = temp.Remove(0, 1).Insert(0, "0");  //000
                        str = new string(speed);
                        String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
                        str = str.Remove(0, 1).Insert(0, binary);
                        SendPacket(str);
                        button3.BackColor = System.Drawing.Color.LightSkyBlue;
                        blade_on.BackColor = SystemColors.Control;
                        textBox2.Text = str;
                    

                }
                if ((this.stateOld.Gamepad.Buttons == GamepadButtonFlags.DPadDown && stateNew.Gamepad.Buttons == GamepadButtonFlags.DPadDown)) // && (LeftTrigger > 0))
                {
                        //Speed_Reset();
                        char[] arr = temp.ToCharArray();
                        arr[1] = '0';   //assuming 0 is backwards
                        arr[2] = '0';
                        temp = new string(arr);
                        String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
                        str = str.Remove(0, 1).Insert(0, binary);
                        SendPacket(str);
                        textBox2.Text = str;
                    

                }
                if ((this.stateOld.Gamepad.Buttons == GamepadButtonFlags.DPadUp && stateNew.Gamepad.Buttons == GamepadButtonFlags.DPadUp)) // && (LeftTrigger > 0))
                {
                    textBox5.Text = str; 
                        //Speed_Reset();
                        char[] arr = temp.ToCharArray();
                        arr[1] = '1';   //assuming one is forward
                        arr[2] = '1';
                        temp = new string(arr);
                        String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
                        str = str.Remove(0, 1).Insert(0, binary);
                        SendPacket(str);
                        textBox2.Text = str;

                    
                }
                if ((this.stateOld.Gamepad.Buttons == GamepadButtonFlags.DPadLeft && stateNew.Gamepad.Buttons == GamepadButtonFlags.DPadLeft)) //&& (LeftTrigger > 0))
                {
                        //Speed_Reset();
                        string motion = str;
                        char[] arr = temp.ToCharArray();
                        char[] speed = motion.ToCharArray();
                        arr[1] = '0';   //assuming one is forward
                        arr[2] = '1';  //assuming is backward

                        speed[1] = '6';
                        speed[2] = '4';
                        speed[3] = '0';
                        speed[4] = '0';
                        temp = new string(arr);
                        motion = new string(speed);
                        String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
                        motion = motion.Remove(0, 1).Insert(0, binary);
                        SendPacket(motion);
                        textBox2.Text = motion;
                    
                }
                if (this.stateOld.Gamepad.Buttons == GamepadButtonFlags.DPadRight && stateNew.Gamepad.Buttons == GamepadButtonFlags.DPadRight)
                {
                        // Speed_Reset();
                        string motion = str;
                        char[] arr = temp.ToCharArray();
                        char[] speed = motion.ToCharArray();
                        arr[1] = '1';   //assuming one is forward
                        arr[2] = '0';  //assuming is backward

                        speed[1] = '0';
                        speed[2] = '0';
                        speed[3] = '6';
                        speed[4] = '4';
                        temp = new string(arr);
                        motion = new string(speed);
                        String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
                        motion = motion.Remove(0, 1).Insert(0, binary);
                        SendPacket(motion);
                        textBox2.Text = motion;
                    
                }
            }
            else
            {
                char[] arr = str.ToCharArray();
                char[] array = temp.ToCharArray();
                array[0] = '0';
               array[1] = '0';
               array[2] = '0';
                arr[1] = '0';
                arr[2] = '0';
                arr[3] = '0';
                arr[4] = '0';
                string test = new string(arr);
                temp = new string(array);
               String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
               str = str.Remove(0, 1).Insert(0, binary);
                SendPacket(test);
                textBox2.Text = test;

            }
        }
        private void button1_Click(object sender, EventArgs e)  //Connect button
        {
            Connect();
            timer1.Enabled = true;
            timer2.Enabled = true;
            
        }
        private void button2_Click(object sender, EventArgs e) //blade on button
        {
            
                char[] speed = str.ToCharArray();
                speed[1] = '0';
                speed[2] = '0';
                speed[3] = '0';
                speed[4] = '0';
                temp = temp.Remove(0, 1).Insert(0, "1");  //100

                str = new string(speed);
                String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
                str = str.Remove(0, 1).Insert(0, binary);
                SendPacket(str);
                textBox2.Text = str;
                button3.BackColor = SystemColors.Control;
                blade_on.BackColor = System.Drawing.Color.LightSkyBlue;
            
        
        }
        private void button3_Click(object sender, EventArgs e) //blade off
        {

            if (temp[0] != '0')
            {
                temp = temp.Remove(0, 1).Insert(0, "0");  //000
                String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);  //convert to decimal
                str = str.Remove(0, 1).Insert(0, binary);
                SendPacket(str);
                button3.BackColor = System.Drawing.Color.LightSkyBlue;
                blade_on.BackColor = SystemColors.Control;
                textBox2.Text = str;
            }
        }
        private void button4_Click(object sender, EventArgs e) //increase speed
        {
            char[] arr = str.ToCharArray();
            string speed;
            string bit_1 = (arr[1]).ToString();
            string bit_2 = (arr[2]).ToString();
            string val = bit_1 + bit_2;    //2 bit hex value to be added
            string dec = Convert.ToString(Convert.ToInt32(val, 16), 10);
            if (Convert.ToInt32(dec) >= 235)                               
            {
                speed = "FF";
                textBox5.Text = "Max Speed";
            }
            else
            {
                int intFromHex = int.Parse(val, System.Globalization.NumberStyles.HexNumber) + 20;  //increment hex value by 20
                textBox5.Text = intFromHex.ToString();
                speed = intFromHex.ToString("X");
            }
            arr[1] = speed[0];
            arr[2] = speed[1];
            arr[3] = speed[0];
            arr[4] = speed[1];
            str = new string(arr);
            SendPacket(str);
            textBox2.Text = str;
        }
        private void button5_Click(object sender, EventArgs e)  //decrease sppeed
        {
            
            char[] arr = str.ToCharArray();
            string speed;
            string bit_1 = (arr[1]).ToString();
            string bit_2 = (arr[2]).ToString();
            string val = bit_1 + bit_2;    //2 bit hex value to be added
            string dec = Convert.ToString(Convert.ToInt32(val, 16), 10);
            if(Convert.ToInt32(dec)<36)
            {
                speed = "00";
                textBox5.Text = "0";
            }
            
            else
            {
                int intFromHex = int.Parse(val, System.Globalization.NumberStyles.HexNumber) - 20;  //decrement hex value by 20
                textBox5.Text = intFromHex.ToString();
                speed = intFromHex.ToString("X");
            }
            arr[1] = speed[0];
            arr[2] = speed[1];
            arr[3] = speed[0];
            arr[4] = speed[1];
            str = new string(arr);
            SendPacket(str);
            textBox2.Text = str;
        }
       

        private void button2_Click_1(object sender, EventArgs e)  //stop button
        {
            char[] arr = str.ToCharArray();
            char[] array = temp.ToCharArray();
            array[0] = '0';
            array[1] = '0';
            array[2] = '0';
            arr[1] = '0';
            arr[2] = '0';
            arr[3] = '0';
            arr[4] = '0';
            str = new string(arr);
            temp = new string(array);
            String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
            str = str.Remove(0, 1).Insert(0, binary);
            SendPacket(str);
            string speed = arr[3].ToString() + arr[4].ToString();
            int dec = int.Parse(speed, System.Globalization.NumberStyles.HexNumber);
            textBox5.Text = dec.ToString();
            textBox2.Text = str;
        }

        private void button6_Click(object sender, EventArgs e)  //start button
        {
            
            char[] decoy = initial.ToCharArray();
            char[] arr = temp.ToCharArray();
            char[] packet = str.ToCharArray();
            arr[0] = '1';
            arr[1] = '1';   //assuming 0 is backwards
            arr[2] = '1';
            temp = new string(arr);

            packet[1] = decoy[1];
            packet[2] = decoy[2];
            packet[3] = decoy[3];
            packet[4] = decoy[4];
            str = new string(packet);
            String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
            str = str.Remove(0, 1).Insert(0, binary);
            SendPacket(str);
            string speed = packet[3].ToString() + packet[4].ToString();
            int dec=int.Parse(speed, System.Globalization.NumberStyles.HexNumber);
            textBox5.Text = dec.ToString();
            textBox2.Text = str;
        }
       /* private void Speed_Reset()
        {
            string reset = str;
            char[] arr = reset.ToCharArray();
            arr[1] = '0';
            arr[2] = '0';
            arr[3] = '0';
            arr[4] = '0';
            reset = new string(arr);
            SendPacket(reset);
            textBox2.Text = reset;
            Stall();
            //SendPacket(str);
            //textBox2.Text = str;
            

        }

       private void Stop()
        {
            char[] arr = str.ToCharArray();
            char[] array = temp.ToCharArray();
            array[0] = '0';
            array[1] = '0';
            array[2] = '0';
            arr[1] = '0';
            arr[2] = '0';
            arr[3] = '0';
            arr[4] = '0';
            string test = new string(arr);
            temp = new string(array);
            String binary = Convert.ToString(Convert.ToInt32(temp, 2), 10);
            str = str.Remove(0, 1).Insert(0, binary);
            SendPacket(test);
            textBox2.Text = test;

        }
        private void Stall()
        {
            var t = Task.Run(async delegate
            {
                await Task.Delay(2000);     //2sec delay
            });
            t.Wait();
        }*/

        private void timer3_Tick(object sender, EventArgs e)
        {
            
            Wifi(); //put in timer
        }


        private void timer1_Tick(object sender, EventArgs e)
        {
            //clock started in button1()
            GetInput();   //50ms
            Navigate();
        }

        private void textBox5_TextChanged(object sender, EventArgs e)
        {

        }

        private void timer2_Tick(object sender, EventArgs e)  //timer for joystick updating
        {
            //clock started in button1()
            // Joystick();
        }
    }
}
