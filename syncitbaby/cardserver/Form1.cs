using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Text;
using System.Security.Cryptography;
using System.IO;



namespace WindowsApplication2
{
	/// <summary>
	/// Zusammenfassung für Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		public CRC32 hash;
		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.TabPage tabPage2;
		public byte[] indata;
		public int readAll = 0;
		/// <summary>
		/// Erforderliche Designervariable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public Thread UdpThread;

		//public int availableBytes = 0;
		//public byte[] indata;

		private CardSimples cardsimples = new CardSimples();

		/* CARD PROTOCOL DEFINITIONS */
		public int availableBytes = 0;
		byte[] cGetCountryCode = { 0x01, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00};
		byte[] cGetASCIISerial = { 0x01, 0x02, 0x00, 0x03, 0x00, 0x00, 0x00};
		byte[] cGetHEXSerial   = { 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00};
		byte[] cGetCamKey384CZ = 
		{
			0x01, 0x02, 0x09, 0x03, 0x00, 0x40, 
			0x18, 0xD7, 0x55, 0x14, 0xC0, 0x83, 0xF1, 0x38, 
			0x39, 0x6F, 0xF2, 0xEC, 0x4F, 0xE3, 0xF1, 0x85, 
			0x01, 0x46, 0x06, 0xCE, 0x7D, 0x08, 0x2C, 0x74, 
			0x46, 0x8F, 0x72, 0xC4, 0xEA, 0xD7, 0x9C, 0xE0, 
			0xE1, 0xFF, 0x58, 0xE7, 0x70, 0x0C, 0x92, 0x45, 
			0x26, 0x18, 0x4F, 0xA0, 0xE2, 0xF5, 0x9E, 0x46, 
			0x6F, 0xAE, 0x95, 0x35, 0xB0, 0x49, 0xB2, 0x0E, 
			0xA4, 0x1F, 0x8E, 0x47, 0xD0, 0x24, 0x11, 0xD0,
			0x00
		};

		byte[] cGetCamKey384DZ =    
		{
			0x01, 0x02, 0x09, 0x03, 0x00, 0x40, 
			0x27, 0xF2, 0xD6, 0xCD, 0xE6, 0x88, 0x62, 0x46, 
			0x81, 0xB0, 0xF5, 0x3E, 0x6F, 0x13, 0x4D, 0xCC, 
			0xFE, 0xD0, 0x67, 0xB1, 0x93, 0xDD, 0xF4, 0xDE, 
			0xEF, 0xF5, 0x3B, 0x04, 0x1D, 0xE5, 0xC3, 0xB2, 
			0x54, 0x38, 0x57, 0x7E, 0xC8, 0x39, 0x07, 0x2E, 
			0xD2, 0xF4, 0x05, 0xAA, 0x15, 0xB5, 0x55, 0x24, 
			0x90, 0xBB, 0x9B, 0x00, 0x96, 0xF0, 0xCB, 0xF1, 
			0x8A, 0x08, 0x7F, 0x0B, 0xB8, 0x79, 0xC3, 0x5D,
			0x00
		};
		//static ushort ACS = 0x0000;
		private byte[] ACS = { 0x00, 0x00 };
		static ushort CAID = 0;
		static byte[] ASCIISerial = {0,0,0,0,0,0,0,0,0,0,0};
		static byte[] HEXSerial = {0, 0, 0, 0, 0, 0, 0, 0};
		public byte[] CamKey = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
		static byte HEXBase =  0;

		private const int sampleUdpPort = 4568;
		private System.Windows.Forms.TabPage tabPage3;
		private System.Windows.Forms.TabPage tabPage4;
		private System.Windows.Forms.TabPage tabPage5;
		private System.Windows.Forms.TabPage tabPage6;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.TextBox camkeyfield;
		private System.Windows.Forms.TextBox znonz;
		private System.Windows.Forms.TextBox hexbasis;
		private System.Windows.Forms.TextBox hexnumber;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Label label4;
		public AxMSCommLib.AxMSComm com;
		private System.Windows.Forms.TextBox typ;
		private System.Windows.Forms.TextBox ASCIISer;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox version;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox ATR;
		private System.Windows.Forms.Button button2;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.ListView listView1;
		private System.Windows.Forms.ColumnHeader TierID;
		private System.Windows.Forms.ColumnHeader DateOn;
		private System.Windows.Forms.ColumnHeader ValidTo;
		private System.Windows.Forms.Button button3;
		private System.Windows.Forms.ColumnHeader Sender;
		private System.Windows.Forms.GroupBox groupBox3;
		private System.Windows.Forms.ListView listView2;
		private System.Windows.Forms.ColumnHeader Uname;
		private System.Windows.Forms.ColumnHeader IPAddr;
		private System.Windows.Forms.ColumnHeader Program;
		private System.Windows.Forms.ColumnHeader Status;
		private System.Windows.Forms.Button killthread;
		public System.Windows.Forms.RichTextBox udpdebug;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.GroupBox groupBox4;
		private System.Windows.Forms.GroupBox groupBox5;
		private System.Windows.Forms.RichTextBox log;
		private System.Windows.Forms.Button button4;
		private System.Windows.Forms.Button button5;
		private System.Windows.Forms.GroupBox groupBox6;
		private System.Windows.Forms.TextBox textBox1;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.CheckedListBox checkedListBox1;
		private System.Windows.Forms.Label label9;
		private System.Windows.Forms.Button button6;
		private System.Windows.Forms.GroupBox groupBox7;
		private System.Windows.Forms.ComboBox comboBox1;
		private System.Windows.Forms.Label label10;
		private System.Windows.Forms.TrackBar trackBar1;
		private System.Windows.Forms.Label label11;
		private System.Windows.Forms.Button button7;
		private System.Windows.Forms.GroupBox groupBox8;
		private System.Windows.Forms.CheckBox checkBox1;
		private System.Windows.Forms.CheckBox checkBox2;
		private System.Windows.Forms.CheckBox checkBox3;
		private System.Windows.Forms.GroupBox groupBox9;
		private System.Windows.Forms.TextBox textBox2;
		private System.Windows.Forms.Label label12;
		private System.Windows.Forms.Button button8;
		private System.Windows.Forms.RadioButton radioButton1;
		private System.Windows.Forms.RadioButton radioButton2;
		private System.Windows.Forms.RadioButton radioButton3;
		private System.Windows.Forms.Button button9;
		private System.Windows.Forms.Button button10;
		private System.Windows.Forms.StatusBar statusBar1;
		private System.Windows.Forms.StatusBarPanel statusBarPanel1;
		private System.Windows.Forms.StatusBarPanel statusBarPanel2;
		private System.Windows.Forms.StatusBarPanel statusBarPanel3;
		private System.Windows.Forms.GroupBox groupBox10;
		private System.Windows.Forms.Label label3;
		
		public Form1()
		{
			//
			// Erforderlich für die Windows Form-Designerunterstützung
			//
			InitializeComponent();
			try 
			{
				InitComPort();
			}
			catch(Exception ComException)
			{
				button2.Enabled = false;
				button3.Enabled = false;
				label3.Text = "Funktion deaktiviert ! Konnte COM-Port nicht öffnen. ";
			}
			//
			// TODO: Fügen Sie den Konstruktorcode nach dem Aufruf von InitializeComponent hinzu
			//
		}

		/// <summary>
		/// Die verwendeten Ressourcen bereinigen.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Vom Windows Form-Designer generierter Code
		/// <summary>
		/// Erforderliche Methode für die Designerunterstützung. 
		/// Der Inhalt der Methode darf nicht mit dem Code-Editor geändert werden.
		/// </summary>
		private void InitializeComponent()
		{
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(Form1));
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this.tabPage2 = new System.Windows.Forms.TabPage();
			this.tabPage3 = new System.Windows.Forms.TabPage();
			this.tabPage4 = new System.Windows.Forms.TabPage();
			this.tabPage5 = new System.Windows.Forms.TabPage();
			this.tabPage6 = new System.Windows.Forms.TabPage();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.listView1 = new System.Windows.Forms.ListView();
			this.TierID = new System.Windows.Forms.ColumnHeader();
			this.Sender = new System.Windows.Forms.ColumnHeader();
			this.DateOn = new System.Windows.Forms.ColumnHeader();
			this.ValidTo = new System.Windows.Forms.ColumnHeader();
			this.button3 = new System.Windows.Forms.Button();
			this.com = new AxMSCommLib.AxMSComm();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.label1 = new System.Windows.Forms.Label();
			this.hexnumber = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.ATR = new System.Windows.Forms.TextBox();
			this.label7 = new System.Windows.Forms.Label();
			this.camkeyfield = new System.Windows.Forms.TextBox();
			this.znonz = new System.Windows.Forms.TextBox();
			this.hexbasis = new System.Windows.Forms.TextBox();
			this.label6 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.typ = new System.Windows.Forms.TextBox();
			this.ASCIISer = new System.Windows.Forms.TextBox();
			this.version = new System.Windows.Forms.TextBox();
			this.button2 = new System.Windows.Forms.Button();
			this.groupBox3 = new System.Windows.Forms.GroupBox();
			this.listView2 = new System.Windows.Forms.ListView();
			this.Uname = new System.Windows.Forms.ColumnHeader();
			this.IPAddr = new System.Windows.Forms.ColumnHeader();
			this.Program = new System.Windows.Forms.ColumnHeader();
			this.Status = new System.Windows.Forms.ColumnHeader();
			this.label3 = new System.Windows.Forms.Label();
			this.killthread = new System.Windows.Forms.Button();
			this.udpdebug = new System.Windows.Forms.RichTextBox();
			this.button1 = new System.Windows.Forms.Button();
			this.groupBox4 = new System.Windows.Forms.GroupBox();
			this.groupBox5 = new System.Windows.Forms.GroupBox();
			this.log = new System.Windows.Forms.RichTextBox();
			this.button4 = new System.Windows.Forms.Button();
			this.button5 = new System.Windows.Forms.Button();
			this.groupBox6 = new System.Windows.Forms.GroupBox();
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.label8 = new System.Windows.Forms.Label();
			this.checkedListBox1 = new System.Windows.Forms.CheckedListBox();
			this.label9 = new System.Windows.Forms.Label();
			this.button6 = new System.Windows.Forms.Button();
			this.groupBox7 = new System.Windows.Forms.GroupBox();
			this.comboBox1 = new System.Windows.Forms.ComboBox();
			this.label10 = new System.Windows.Forms.Label();
			this.trackBar1 = new System.Windows.Forms.TrackBar();
			this.label11 = new System.Windows.Forms.Label();
			this.button7 = new System.Windows.Forms.Button();
			this.groupBox8 = new System.Windows.Forms.GroupBox();
			this.checkBox1 = new System.Windows.Forms.CheckBox();
			this.checkBox2 = new System.Windows.Forms.CheckBox();
			this.checkBox3 = new System.Windows.Forms.CheckBox();
			this.groupBox9 = new System.Windows.Forms.GroupBox();
			this.textBox2 = new System.Windows.Forms.TextBox();
			this.label12 = new System.Windows.Forms.Label();
			this.button8 = new System.Windows.Forms.Button();
			this.radioButton1 = new System.Windows.Forms.RadioButton();
			this.radioButton2 = new System.Windows.Forms.RadioButton();
			this.radioButton3 = new System.Windows.Forms.RadioButton();
			this.button9 = new System.Windows.Forms.Button();
			this.button10 = new System.Windows.Forms.Button();
			this.statusBar1 = new System.Windows.Forms.StatusBar();
			this.statusBarPanel1 = new System.Windows.Forms.StatusBarPanel();
			this.statusBarPanel2 = new System.Windows.Forms.StatusBarPanel();
			this.statusBarPanel3 = new System.Windows.Forms.StatusBarPanel();
			this.groupBox10 = new System.Windows.Forms.GroupBox();
			this.tabControl1.SuspendLayout();
			this.tabPage1.SuspendLayout();
			this.tabPage2.SuspendLayout();
			this.tabPage3.SuspendLayout();
			this.tabPage6.SuspendLayout();
			this.groupBox2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.com)).BeginInit();
			this.groupBox1.SuspendLayout();
			this.groupBox3.SuspendLayout();
			this.groupBox5.SuspendLayout();
			this.groupBox6.SuspendLayout();
			this.groupBox7.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.trackBar1)).BeginInit();
			this.groupBox8.SuspendLayout();
			this.groupBox9.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.statusBarPanel1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.statusBarPanel2)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.statusBarPanel3)).BeginInit();
			this.SuspendLayout();
			// 
			// tabControl1
			// 
			this.tabControl1.Controls.Add(this.tabPage1);
			this.tabControl1.Controls.Add(this.tabPage2);
			this.tabControl1.Controls.Add(this.tabPage3);
			this.tabControl1.Controls.Add(this.tabPage4);
			this.tabControl1.Controls.Add(this.tabPage5);
			this.tabControl1.Controls.Add(this.tabPage6);
			this.tabControl1.Location = new System.Drawing.Point(0, 0);
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(480, 432);
			this.tabControl1.TabIndex = 7;
			// 
			// tabPage1
			// 
			this.tabPage1.Controls.Add(this.groupBox4);
			this.tabPage1.Controls.Add(this.groupBox3);
			this.tabPage1.Location = new System.Drawing.Point(4, 22);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Size = new System.Drawing.Size(472, 406);
			this.tabPage1.TabIndex = 0;
			this.tabPage1.Text = "Aktivität";
			// 
			// tabPage2
			// 
			this.tabPage2.Controls.Add(this.groupBox5);
			this.tabPage2.Location = new System.Drawing.Point(4, 22);
			this.tabPage2.Name = "tabPage2";
			this.tabPage2.Size = new System.Drawing.Size(472, 406);
			this.tabPage2.TabIndex = 1;
			this.tabPage2.Text = "Log";
			// 
			// tabPage3
			// 
			this.tabPage3.Controls.Add(this.groupBox10);
			this.tabPage3.Controls.Add(this.groupBox9);
			this.tabPage3.Controls.Add(this.groupBox8);
			this.tabPage3.Controls.Add(this.groupBox7);
			this.tabPage3.Controls.Add(this.groupBox6);
			this.tabPage3.Controls.Add(this.udpdebug);
			this.tabPage3.Location = new System.Drawing.Point(4, 22);
			this.tabPage3.Name = "tabPage3";
			this.tabPage3.Size = new System.Drawing.Size(472, 406);
			this.tabPage3.TabIndex = 2;
			this.tabPage3.Text = "Einstellungen";
			// 
			// tabPage4
			// 
			this.tabPage4.Location = new System.Drawing.Point(4, 22);
			this.tabPage4.Name = "tabPage4";
			this.tabPage4.Size = new System.Drawing.Size(472, 398);
			this.tabPage4.TabIndex = 3;
			this.tabPage4.Text = "CW-Cache";
			// 
			// tabPage5
			// 
			this.tabPage5.Location = new System.Drawing.Point(4, 22);
			this.tabPage5.Name = "tabPage5";
			this.tabPage5.Size = new System.Drawing.Size(472, 398);
			this.tabPage5.TabIndex = 4;
			this.tabPage5.Text = "Benutzer";
			// 
			// tabPage6
			// 
			this.tabPage6.Controls.Add(this.groupBox2);
			this.tabPage6.Controls.Add(this.groupBox1);
			this.tabPage6.Location = new System.Drawing.Point(4, 22);
			this.tabPage6.Name = "tabPage6";
			this.tabPage6.Size = new System.Drawing.Size(472, 398);
			this.tabPage6.TabIndex = 5;
			this.tabPage6.Text = "Kartendetails";
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.listView1);
			this.groupBox2.Controls.Add(this.button3);
			this.groupBox2.Controls.Add(this.com);
			this.groupBox2.Location = new System.Drawing.Point(16, 176);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(440, 208);
			this.groupBox2.TabIndex = 45;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "TierIDs";
			// 
			// listView1
			// 
			this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																						this.TierID,
																						this.Sender,
																						this.DateOn,
																						this.ValidTo});
			this.listView1.GridLines = true;
			this.listView1.Location = new System.Drawing.Point(16, 24);
			this.listView1.Name = "listView1";
			this.listView1.Size = new System.Drawing.Size(408, 144);
			this.listView1.TabIndex = 0;
			this.listView1.View = System.Windows.Forms.View.Details;
			// 
			// TierID
			// 
			this.TierID.Text = "TierID";
			this.TierID.Width = 65;
			// 
			// Sender
			// 
			this.Sender.Text = "Sendername";
			this.Sender.Width = 120;
			// 
			// DateOn
			// 
			this.DateOn.Text = "Date On";
			this.DateOn.Width = 80;
			// 
			// ValidTo
			// 
			this.ValidTo.Text = "Valid To";
			this.ValidTo.Width = 80;
			// 
			// button3
			// 
			this.button3.Location = new System.Drawing.Point(352, 176);
			this.button3.Name = "button3";
			this.button3.TabIndex = 26;
			this.button3.Text = "Refresh";
			// 
			// com
			// 
			this.com.ContainingControl = this;
			this.com.Enabled = true;
			this.com.Location = new System.Drawing.Point(8, 168);
			this.com.Name = "com";
			this.com.OcxState = ((System.Windows.Forms.AxHost.State)(resources.GetObject("com.OcxState")));
			this.com.Size = new System.Drawing.Size(38, 38);
			this.com.TabIndex = 35;
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.label3);
			this.groupBox1.Controls.Add(this.label1);
			this.groupBox1.Controls.Add(this.hexnumber);
			this.groupBox1.Controls.Add(this.label2);
			this.groupBox1.Controls.Add(this.ATR);
			this.groupBox1.Controls.Add(this.label7);
			this.groupBox1.Controls.Add(this.camkeyfield);
			this.groupBox1.Controls.Add(this.znonz);
			this.groupBox1.Controls.Add(this.hexbasis);
			this.groupBox1.Controls.Add(this.label6);
			this.groupBox1.Controls.Add(this.label5);
			this.groupBox1.Controls.Add(this.label4);
			this.groupBox1.Controls.Add(this.typ);
			this.groupBox1.Controls.Add(this.ASCIISer);
			this.groupBox1.Controls.Add(this.version);
			this.groupBox1.Controls.Add(this.button2);
			this.groupBox1.Location = new System.Drawing.Point(16, 16);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(440, 144);
			this.groupBox1.TabIndex = 44;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "KartenInfo";
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(16, 24);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(32, 16);
			this.label1.TabIndex = 27;
			this.label1.Text = "ATR";
			// 
			// hexnumber
			// 
			this.hexnumber.Location = new System.Drawing.Point(152, 80);
			this.hexnumber.Name = "hexnumber";
			this.hexnumber.Size = new System.Drawing.Size(56, 20);
			this.hexnumber.TabIndex = 39;
			this.hexnumber.Text = "";
			this.hexnumber.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(376, 24);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(32, 16);
			this.label2.TabIndex = 30;
			this.label2.Text = "ACS";
			// 
			// ATR
			// 
			this.ATR.Location = new System.Drawing.Point(16, 40);
			this.ATR.Name = "ATR";
			this.ATR.Size = new System.Drawing.Size(352, 20);
			this.ATR.TabIndex = 26;
			this.ATR.Text = "";
			this.ATR.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// label7
			// 
			this.label7.Location = new System.Drawing.Point(280, 64);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(96, 16);
			this.label7.TabIndex = 43;
			this.label7.Text = "CamKey";
			// 
			// camkeyfield
			// 
			this.camkeyfield.Location = new System.Drawing.Point(280, 80);
			this.camkeyfield.Name = "camkeyfield";
			this.camkeyfield.Size = new System.Drawing.Size(136, 20);
			this.camkeyfield.TabIndex = 42;
			this.camkeyfield.Text = "";
			this.camkeyfield.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// znonz
			// 
			this.znonz.Location = new System.Drawing.Point(120, 80);
			this.znonz.Name = "znonz";
			this.znonz.Size = new System.Drawing.Size(16, 20);
			this.znonz.TabIndex = 41;
			this.znonz.Text = "";
			this.znonz.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// hexbasis
			// 
			this.hexbasis.Location = new System.Drawing.Point(224, 80);
			this.hexbasis.Name = "hexbasis";
			this.hexbasis.Size = new System.Drawing.Size(24, 20);
			this.hexbasis.TabIndex = 40;
			this.hexbasis.Text = "";
			this.hexbasis.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// label6
			// 
			this.label6.Location = new System.Drawing.Point(224, 64);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(56, 16);
			this.label6.TabIndex = 38;
			this.label6.Text = "HexBase";
			// 
			// label5
			// 
			this.label5.Location = new System.Drawing.Point(152, 64);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(56, 16);
			this.label5.TabIndex = 37;
			this.label5.Text = "Hex";
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(40, 64);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(80, 16);
			this.label4.TabIndex = 36;
			this.label4.Text = "Seriennummer";
			// 
			// typ
			// 
			this.typ.Location = new System.Drawing.Point(16, 80);
			this.typ.Name = "typ";
			this.typ.Size = new System.Drawing.Size(16, 20);
			this.typ.TabIndex = 34;
			this.typ.Text = "";
			this.typ.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// ASCIISer
			// 
			this.ASCIISer.Location = new System.Drawing.Point(40, 80);
			this.ASCIISer.Name = "ASCIISer";
			this.ASCIISer.Size = new System.Drawing.Size(72, 20);
			this.ASCIISer.TabIndex = 33;
			this.ASCIISer.Text = "";
			this.ASCIISer.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// version
			// 
			this.version.Location = new System.Drawing.Point(376, 40);
			this.version.Name = "version";
			this.version.Size = new System.Drawing.Size(40, 20);
			this.version.TabIndex = 29;
			this.version.Text = "";
			this.version.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// button2
			// 
			this.button2.Location = new System.Drawing.Point(344, 112);
			this.button2.Name = "button2";
			this.button2.TabIndex = 25;
			this.button2.Text = "Reset";
			this.button2.Click += new System.EventHandler(this.button2_Click);
			// 
			// groupBox3
			// 
			this.groupBox3.Controls.Add(this.listView2);
			this.groupBox3.Location = new System.Drawing.Point(16, 16);
			this.groupBox3.Name = "groupBox3";
			this.groupBox3.Size = new System.Drawing.Size(440, 280);
			this.groupBox3.TabIndex = 0;
			this.groupBox3.TabStop = false;
			this.groupBox3.Text = "Aktive Benutzer";
			// 
			// listView2
			// 
			this.listView2.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																						this.Uname,
																						this.IPAddr,
																						this.Program,
																						this.Status});
			this.listView2.GridLines = true;
			this.listView2.Location = new System.Drawing.Point(16, 24);
			this.listView2.Name = "listView2";
			this.listView2.Size = new System.Drawing.Size(408, 248);
			this.listView2.TabIndex = 0;
			this.listView2.View = System.Windows.Forms.View.Details;
			// 
			// Uname
			// 
			this.Uname.Text = "Benutzer";
			this.Uname.Width = 80;
			// 
			// IPAddr
			// 
			this.IPAddr.Text = "IP-Adresse";
			this.IPAddr.Width = 80;
			// 
			// Program
			// 
			this.Program.Text = "Sender";
			this.Program.Width = 120;
			// 
			// Status
			// 
			this.Status.Text = "Status";
			this.Status.Width = 120;
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(40, 118);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(288, 16);
			this.label3.TabIndex = 44;
			// 
			// killthread
			// 
			this.killthread.Location = new System.Drawing.Point(110, 435);
			this.killthread.Name = "killthread";
			this.killthread.Size = new System.Drawing.Size(96, 20);
			this.killthread.TabIndex = 9;
			this.killthread.Text = "Stop Server";
			this.killthread.Click += new System.EventHandler(this.killthread_Click_1);
			// 
			// udpdebug
			// 
			this.udpdebug.Location = new System.Drawing.Point(280, 320);
			this.udpdebug.Name = "udpdebug";
			this.udpdebug.Size = new System.Drawing.Size(168, 64);
			this.udpdebug.TabIndex = 7;
			this.udpdebug.Text = "";
			// 
			// button1
			// 
			this.button1.Location = new System.Drawing.Point(6, 435);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(96, 20);
			this.button1.TabIndex = 5;
			this.button1.Text = "Start Server";
			this.button1.Click += new System.EventHandler(this.button1_Click_1);
			// 
			// groupBox4
			// 
			this.groupBox4.Location = new System.Drawing.Point(16, 304);
			this.groupBox4.Name = "groupBox4";
			this.groupBox4.Size = new System.Drawing.Size(440, 80);
			this.groupBox4.TabIndex = 1;
			this.groupBox4.TabStop = false;
			this.groupBox4.Text = "Kartenüberblick";
			// 
			// groupBox5
			// 
			this.groupBox5.Controls.Add(this.log);
			this.groupBox5.Controls.Add(this.button4);
			this.groupBox5.Controls.Add(this.button5);
			this.groupBox5.Location = new System.Drawing.Point(16, 16);
			this.groupBox5.Name = "groupBox5";
			this.groupBox5.Size = new System.Drawing.Size(440, 368);
			this.groupBox5.TabIndex = 0;
			this.groupBox5.TabStop = false;
			this.groupBox5.Text = "Ereignislog";
			// 
			// log
			// 
			this.log.BackColor = System.Drawing.Color.DarkSeaGreen;
			this.log.Location = new System.Drawing.Point(16, 24);
			this.log.Name = "log";
			this.log.Size = new System.Drawing.Size(408, 304);
			this.log.TabIndex = 0;
			this.log.Text = "";
			// 
			// button4
			// 
			this.button4.Location = new System.Drawing.Point(240, 336);
			this.button4.Name = "button4";
			this.button4.Size = new System.Drawing.Size(85, 23);
			this.button4.TabIndex = 1;
			this.button4.Text = "Log löschen";
			// 
			// button5
			// 
			this.button5.Location = new System.Drawing.Point(336, 336);
			this.button5.Name = "button5";
			this.button5.Size = new System.Drawing.Size(85, 23);
			this.button5.TabIndex = 2;
			this.button5.Text = "Log speichern";
			// 
			// groupBox6
			// 
			this.groupBox6.Controls.Add(this.button6);
			this.groupBox6.Controls.Add(this.label9);
			this.groupBox6.Controls.Add(this.checkedListBox1);
			this.groupBox6.Controls.Add(this.label8);
			this.groupBox6.Controls.Add(this.textBox1);
			this.groupBox6.Location = new System.Drawing.Point(16, 16);
			this.groupBox6.Name = "groupBox6";
			this.groupBox6.Size = new System.Drawing.Size(208, 96);
			this.groupBox6.TabIndex = 10;
			this.groupBox6.TabStop = false;
			this.groupBox6.Text = "UDP";
			// 
			// textBox1
			// 
			this.textBox1.Location = new System.Drawing.Point(16, 32);
			this.textBox1.Name = "textBox1";
			this.textBox1.Size = new System.Drawing.Size(48, 20);
			this.textBox1.TabIndex = 0;
			this.textBox1.Text = "20248";
			this.textBox1.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// label8
			// 
			this.label8.Location = new System.Drawing.Point(16, 16);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(48, 16);
			this.label8.TabIndex = 1;
			this.label8.Text = "Port";
			// 
			// checkedListBox1
			// 
			this.checkedListBox1.Items.AddRange(new object[] {
																 "127.0.0.1"});
			this.checkedListBox1.Location = new System.Drawing.Point(80, 32);
			this.checkedListBox1.Name = "checkedListBox1";
			this.checkedListBox1.Size = new System.Drawing.Size(120, 49);
			this.checkedListBox1.TabIndex = 3;
			// 
			// label9
			// 
			this.label9.Location = new System.Drawing.Point(80, 16);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(24, 16);
			this.label9.TabIndex = 4;
			this.label9.Text = "IPs";
			// 
			// button6
			// 
			this.button6.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.button6.Location = new System.Drawing.Point(16, 64);
			this.button6.Name = "button6";
			this.button6.Size = new System.Drawing.Size(48, 16);
			this.button6.TabIndex = 5;
			this.button6.Text = "Sumbit";
			// 
			// groupBox7
			// 
			this.groupBox7.Controls.Add(this.label11);
			this.groupBox7.Controls.Add(this.trackBar1);
			this.groupBox7.Controls.Add(this.label10);
			this.groupBox7.Controls.Add(this.comboBox1);
			this.groupBox7.Controls.Add(this.button7);
			this.groupBox7.Location = new System.Drawing.Point(240, 16);
			this.groupBox7.Name = "groupBox7";
			this.groupBox7.Size = new System.Drawing.Size(216, 96);
			this.groupBox7.TabIndex = 11;
			this.groupBox7.TabStop = false;
			this.groupBox7.Text = "Kartenleser";
			// 
			// comboBox1
			// 
			this.comboBox1.Items.AddRange(new object[] {
														   "COM1",
														   "COM2",
														   "COM3",
														   "COM4",
														   "COM5"});
			this.comboBox1.Location = new System.Drawing.Point(8, 32);
			this.comboBox1.Name = "comboBox1";
			this.comboBox1.Size = new System.Drawing.Size(64, 21);
			this.comboBox1.TabIndex = 0;
			// 
			// label10
			// 
			this.label10.Location = new System.Drawing.Point(8, 16);
			this.label10.Name = "label10";
			this.label10.Size = new System.Drawing.Size(56, 16);
			this.label10.TabIndex = 1;
			this.label10.Text = "COM-Port";
			// 
			// trackBar1
			// 
			this.trackBar1.Location = new System.Drawing.Point(96, 32);
			this.trackBar1.Maximum = 20;
			this.trackBar1.Name = "trackBar1";
			this.trackBar1.TabIndex = 2;
			this.trackBar1.Value = 10;
			// 
			// label11
			// 
			this.label11.Location = new System.Drawing.Point(104, 16);
			this.label11.Name = "label11";
			this.label11.Size = new System.Drawing.Size(72, 16);
			this.label11.TabIndex = 3;
			this.label11.Text = "ATR-Timeout";
			// 
			// button7
			// 
			this.button7.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.button7.Location = new System.Drawing.Point(16, 64);
			this.button7.Name = "button7";
			this.button7.Size = new System.Drawing.Size(48, 16);
			this.button7.TabIndex = 6;
			this.button7.Text = "Sumbit";
			// 
			// groupBox8
			// 
			this.groupBox8.Controls.Add(this.label12);
			this.groupBox8.Controls.Add(this.textBox2);
			this.groupBox8.Controls.Add(this.checkBox3);
			this.groupBox8.Controls.Add(this.checkBox2);
			this.groupBox8.Controls.Add(this.checkBox1);
			this.groupBox8.Controls.Add(this.button8);
			this.groupBox8.Location = new System.Drawing.Point(16, 128);
			this.groupBox8.Name = "groupBox8";
			this.groupBox8.Size = new System.Drawing.Size(208, 80);
			this.groupBox8.TabIndex = 12;
			this.groupBox8.TabStop = false;
			this.groupBox8.Text = "CAID-Filter";
			// 
			// checkBox1
			// 
			this.checkBox1.Location = new System.Drawing.Point(16, 14);
			this.checkBox1.Name = "checkBox1";
			this.checkBox1.Size = new System.Drawing.Size(56, 24);
			this.checkBox1.TabIndex = 0;
			this.checkBox1.Text = "1702";
			// 
			// checkBox2
			// 
			this.checkBox2.Location = new System.Drawing.Point(16, 52);
			this.checkBox2.Name = "checkBox2";
			this.checkBox2.Size = new System.Drawing.Size(56, 24);
			this.checkBox2.TabIndex = 1;
			this.checkBox2.Text = "1722";
			// 
			// checkBox3
			// 
			this.checkBox3.Location = new System.Drawing.Point(16, 32);
			this.checkBox3.Name = "checkBox3";
			this.checkBox3.Size = new System.Drawing.Size(88, 24);
			this.checkBox3.TabIndex = 2;
			this.checkBox3.Text = "1702 / 1722";
			// 
			// groupBox9
			// 
			this.groupBox9.Controls.Add(this.radioButton3);
			this.groupBox9.Controls.Add(this.radioButton2);
			this.groupBox9.Controls.Add(this.radioButton1);
			this.groupBox9.Controls.Add(this.button9);
			this.groupBox9.Controls.Add(this.button10);
			this.groupBox9.Location = new System.Drawing.Point(240, 128);
			this.groupBox9.Name = "groupBox9";
			this.groupBox9.Size = new System.Drawing.Size(216, 80);
			this.groupBox9.TabIndex = 13;
			this.groupBox9.TabStop = false;
			this.groupBox9.Text = "EMM-Strings";
			// 
			// textBox2
			// 
			this.textBox2.Location = new System.Drawing.Point(120, 24);
			this.textBox2.Name = "textBox2";
			this.textBox2.Size = new System.Drawing.Size(64, 20);
			this.textBox2.TabIndex = 3;
			this.textBox2.Text = "";
			// 
			// label12
			// 
			this.label12.Location = new System.Drawing.Point(120, 8);
			this.label12.Name = "label12";
			this.label12.Size = new System.Drawing.Size(64, 16);
			this.label12.TabIndex = 4;
			this.label12.Text = "Custom";
			// 
			// button8
			// 
			this.button8.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.button8.Location = new System.Drawing.Point(128, 56);
			this.button8.Name = "button8";
			this.button8.Size = new System.Drawing.Size(48, 16);
			this.button8.TabIndex = 6;
			this.button8.Text = "Sumbit";
			// 
			// radioButton1
			// 
			this.radioButton1.Location = new System.Drawing.Point(8, 14);
			this.radioButton1.Name = "radioButton1";
			this.radioButton1.Size = new System.Drawing.Size(72, 24);
			this.radioButton1.TabIndex = 0;
			this.radioButton1.Text = "D0 (Date)";
			// 
			// radioButton2
			// 
			this.radioButton2.Location = new System.Drawing.Point(8, 32);
			this.radioButton2.Name = "radioButton2";
			this.radioButton2.Size = new System.Drawing.Size(80, 26);
			this.radioButton2.TabIndex = 1;
			this.radioButton2.Text = "D2 (ChID)";
			// 
			// radioButton3
			// 
			this.radioButton3.Location = new System.Drawing.Point(8, 52);
			this.radioButton3.Name = "radioButton3";
			this.radioButton3.Size = new System.Drawing.Size(120, 24);
			this.radioButton3.TabIndex = 2;
			this.radioButton3.Text = "D3 (ChID/Counter)";
			// 
			// button9
			// 
			this.button9.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.button9.Location = new System.Drawing.Point(120, 24);
			this.button9.Name = "button9";
			this.button9.Size = new System.Drawing.Size(88, 18);
			this.button9.TabIndex = 7;
			this.button9.Text = "Renotify Clients";
			// 
			// button10
			// 
			this.button10.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.button10.Location = new System.Drawing.Point(136, 48);
			this.button10.Name = "button10";
			this.button10.Size = new System.Drawing.Size(48, 16);
			this.button10.TabIndex = 7;
			this.button10.Text = "Sumbit";
			// 
			// statusBar1
			// 
			this.statusBar1.Location = new System.Drawing.Point(0, 459);
			this.statusBar1.Name = "statusBar1";
			this.statusBar1.Panels.AddRange(new System.Windows.Forms.StatusBarPanel[] {
																						  this.statusBarPanel1,
																						  this.statusBarPanel2,
																						  this.statusBarPanel3});
			this.statusBar1.ShowPanels = true;
			this.statusBar1.Size = new System.Drawing.Size(480, 18);
			this.statusBar1.TabIndex = 10;
			this.statusBar1.Text = "statusBar1";
			// 
			// statusBarPanel1
			// 
			this.statusBarPanel1.Text = "Status";
			this.statusBarPanel1.Width = 150;
			// 
			// statusBarPanel2
			// 
			this.statusBarPanel2.Text = "Zeit";
			this.statusBarPanel2.Width = 150;
			// 
			// statusBarPanel3
			// 
			this.statusBarPanel3.Text = "Kurzdebug";
			this.statusBarPanel3.Width = 160;
			// 
			// groupBox10
			// 
			this.groupBox10.Location = new System.Drawing.Point(16, 224);
			this.groupBox10.Name = "groupBox10";
			this.groupBox10.Size = new System.Drawing.Size(208, 168);
			this.groupBox10.TabIndex = 14;
			this.groupBox10.TabStop = false;
			this.groupBox10.Text = "Verschiedenes";
			// 
			// Form1
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(480, 477);
			this.Controls.Add(this.statusBar1);
			this.Controls.Add(this.tabControl1);
			this.Controls.Add(this.button1);
			this.Controls.Add(this.killthread);
			this.Name = "Form1";
			this.Text = "CW-Abo-Server by Sky & Uska (C) 2005";
			this.tabControl1.ResumeLayout(false);
			this.tabPage1.ResumeLayout(false);
			this.tabPage2.ResumeLayout(false);
			this.tabPage3.ResumeLayout(false);
			this.tabPage6.ResumeLayout(false);
			this.groupBox2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.com)).EndInit();
			this.groupBox1.ResumeLayout(false);
			this.groupBox3.ResumeLayout(false);
			this.groupBox5.ResumeLayout(false);
			this.groupBox6.ResumeLayout(false);
			this.groupBox7.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.trackBar1)).EndInit();
			this.groupBox8.ResumeLayout(false);
			this.groupBox9.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.statusBarPanel1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.statusBarPanel2)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.statusBarPanel3)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion
		/// <summary>
		/// Der Haupteinstiegspunkt für die Anwendung.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
		}



		private void button2_Click(object sender, System.EventArgs e)
		{
			ATR.Clear(); // clear ATR field
			initCard();
		}

		private string byteToString(byte[] data, int length)
		{
			int i = 0;
			string String = "";
			for(i=0;i<length;i++)
			{
				String += String.Format("{0:x2}", data[i]).ToUpper();
				if(i!=length-1)
					String += " ";
			}
			return String;
		}

		unsafe void initCard()
		{
			int i = 0;

			//lets reset the card ...
			com.RTSEnable = true;
			System.Threading.Thread.Sleep(2);
			com.RTSEnable = false;

			System.Threading.Thread.Sleep(50);
			availableBytes = com.InBufferCount;
			indata = (byte[]) com.Input;
			
			ATR.AppendText(byteToString(indata,availableBytes));


			//Get CoCo
			//System.Threading.Thread.Sleep(50);
			cGetCountryCode[6] = cardsimples.XorSum(cGetCountryCode, 6);
			com.Output = cGetCountryCode;			
			System.Threading.Thread.Sleep(150);

			availableBytes = com.InBufferCount;
			indata = (byte[]) com.Input;

			Array.Copy(indata,(cGetCountryCode.Length+8),ACS,0,2);
			version.AppendText(byteToString(ACS,2));

			/*
			ACS = indata[8+cGetCountryCode.Length];
			ACS <<= 8;
			ACS += indata[9+cGetCountryCode.Length];
			version.AppendText(Convert.ToString(ACS,16));
			*/

			CAID = (ushort)indata[13+cGetCountryCode.Length];
			CAID <<= 8;
			CAID += (ushort)indata[14+cGetCountryCode.Length];
			if(CAID==0x1702)
				typ.AppendText("C");
			else if(CAID==0x1722)
				typ.AppendText("D");

			//Get ASCII Serial ...
			cGetASCIISerial[6] = cardsimples.XorSum(cGetASCIISerial, 6);
			com.Output = cGetASCIISerial;
			System.Threading.Thread.Sleep(150);

			availableBytes = com.InBufferCount;
			indata = (byte[]) com.Input;
			for(i=0;i<11;i++)
			{
				ASCIISerial[i]=indata[15+i];
				ASCIISer.AppendText(Convert.ToChar(ASCIISerial[i]) + "");
			}


			cGetHEXSerial[6] = cardsimples.XorSum(cGetHEXSerial, 6);
			com.Output = cGetHEXSerial;
			System.Threading.Thread.Sleep(150);

			availableBytes = com.InBufferCount;
			indata = (byte[]) com.Input;
			//hexnumber.AppendText(Convert.ToString(availableBytes));

			Array.Copy(indata,cGetHEXSerial.Length+20,HEXSerial,0,3);
			HEXBase = indata[cGetHEXSerial.Length+23];

			for(i=0;i<3;i++)
				hexnumber.AppendText(Convert.ToString(HEXSerial[i],16).ToUpper() + " ");

			hexbasis.AppendText(Convert.ToString(HEXBase,16).ToUpper());

			switch (CAID)
			{
				case 0x1702:
				{
					cGetCamKey384CZ[70] = cardsimples.XorSum(cGetCamKey384CZ, 70);
					com.Output = cGetCamKey384CZ;
					System.Threading.Thread.Sleep(4000);

					availableBytes = com.InBufferCount;
					indata = (byte[]) com.Input;
					break;
				}
				default:
				{
					cGetCamKey384DZ[70] = cardsimples.XorSum(cGetCamKey384DZ, 70);
					com.Output = cGetCamKey384DZ;
					System.Threading.Thread.Sleep(4000);

					availableBytes = com.InBufferCount;
					indata = (byte[]) com.Input;
					break;
				}
			}
			if (availableBytes != (17 + cGetCamKey384DZ.Length)) 
			{
				camkeyfield.AppendText(Convert.ToString(availableBytes) + " ");
				camkeyfield.AppendText("CamKey not set! ERROR ");
			} 
			else 
			{
				for(i=0;i<8;i++)
					camkeyfield.AppendText(Convert.ToString(CamKey[i],16).ToUpper() + " ");
                    znonz.AppendText("Z");
			}


		}

		public byte[] request_controlword(byte[] cardstring)
		{

			byte[] key1 = new byte[8];
			byte[] key2 = new byte[8];
			byte[] cw   = new byte[16];

			//Form1 cardcomm = new Form1(8);

			CardSimples cardcrypt = new CardSimples();
			//Form1 cardcomm = new Form1();

			indata = (byte[]) com.Input;

			com.Output = cardstring;	
			System.Threading.Thread.Sleep(350);

			availableBytes = com.InBufferCount;
			indata = (byte[]) com.Input;

			if((indata[2+cardstring.Length]==0x9D) && (indata[3+cardstring.Length]==0x00))
			{
				Array.Copy(indata,14+cardstring.Length,key1,0,8);
				Array.Copy(indata,22+cardstring.Length,key2,0,8);

				cardcrypt.ReverseSessionKeyCrypt(CamKey,key1);
				cardcrypt.ReverseSessionKeyCrypt(CamKey,key2);

				Array.Copy(key1,0,cw,0,8);
				Array.Copy(key2,0,cw,8,8);

			}

			return cw;

		}

		private void button1_Click_1(object sender, System.EventArgs e)
		{
			//UdpServer cwserver = new UdpServer();

			try
			{		
				//Starting the UDP Server thread.
				UdpThread = new Thread(new ThreadStart(StartUdpServer));
				UdpThread.Start();
				udpdebug.AppendText("Started SampleTcpUdpServer's UDP Receiver Thread!\n");
			}
			catch (Exception f)
			{
				udpdebug.AppendText("An UDP Exception has occurred!" + f.ToString());
				UdpThread.Abort();
			}

		}

		private void clear_Click(object sender, System.EventArgs e)
		{
			log.Clear();
		}

		private void killthread_Click_1(object sender, System.EventArgs e)
		{
			try 
			{
				UdpThread.Abort();
			}
			catch(Exception r) { ; }
		}

		public void InitComPort()
		{
			// Set the com port to be 1
			com.CommPort = 2;
    
			// This port is already open, close it to reset it.
			if (com.PortOpen) com.PortOpen = false;
    
			// Trigger the OnComm event whenever data is received
			com.RThreshold = 1;  
    
			// Set the port to 9600 baud, no parity bit, 8 data bits, 1 stop bit (all standard)
			com.Settings = "9600,n,8,2";

			// Force the DTR line high, used sometimes to hang up modems
			com.DTREnable = true;
    
			// No handshaking is used
			com.Handshaking = MSCommLib.HandshakeConstants.comNone;

			// Don't mess with byte arrays, only works with simple data (characters A-Z and numbers)
			//com.InputMode = MSCommLib.InputModeConstants.comInputModeText;
    
			// Use this line instead for byte array input, best for most communications
			com.InputMode = MSCommLib.InputModeConstants.comInputModeBinary;
    
			// Read the entire waiting data when com.Input is used
			com.InputLen = 0;

			// Don't discard nulls, 0x00 is a useful byte
			com.NullDiscard = false;
    
			// Attach the event handler
			com.OnComm += new System.EventHandler(this.OnComm);
    
			// Open the com port
			com.PortOpen = true;  
		}

		public void OnComm(object sender, EventArgs e)  //  MSCommLib OnComm Event Handler
		{
			availableBytes = com.InBufferCount;
			if(availableBytes != 0) 
			{
				; // nothing to do here ...
			}
			//System.Threading.Thread.Sleep(30);

		}

		public void StartUdpServer()
		{
			CWServer cwserver = new CWServer();
			CardComm cardcomm = new CardComm();

			byte[] usernamecrc = new byte[4];
			byte[] cryptedmessage = new byte[112];
			byte[] decryptedmessage = new byte[112];
			byte[] cardrequest = new byte[85];
			byte[] controlword = new byte[16];

			int kundennummer = 0;
			
			IPHostEntry localHostEntry;
			int i=0;
		
			try
			{
				//Create a UDP socket.
				Socket soUdp = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
				
				try
				{
					localHostEntry = Dns.GetHostByName(Dns.GetHostName());
				}
				catch(Exception)
				{
					//udpdebug.AppendText("Local Host not found"); // fail
					return ;
				}
				
			
				IPEndPoint localIpEndPoint = new IPEndPoint(localHostEntry.AddressList[0], sampleUdpPort);
				soUdp.Bind(localIpEndPoint);
			
				while (true)
				{
					byte[] received = new byte[1024];
					IPEndPoint tmpIpEndPoint = new IPEndPoint(localHostEntry.AddressList[0], sampleUdpPort);
					EndPoint remoteEP = (tmpIpEndPoint);
					int bytesReceived = soUdp.ReceiveFrom(received, ref remoteEP);
					//String dataReceived = System.Text.Encoding.ASCII.GetString(received);

					//log.AppendText(dataReceived);

					//listView2.Items.Insert(0, "root");
					//listView2.Items[0].SubItems.Add("haha");

					Array.Copy(received,0,usernamecrc,0,4);
					Array.Copy(received,4,cryptedmessage,0,112);


					if((kundennummer=cwserver.auth_client(usernamecrc))>-1)
					{
						decryptedmessage = cwserver.decrypt_message(cryptedmessage, cwserver.kundendb[kundennummer].pwd, 112);

						if(cwserver.validate_request(decryptedmessage)==1)
						{
							if(cwserver.validate_abo(decryptedmessage,3)==1)
							{
								cardrequest = cwserver.generate_cardrequest(decryptedmessage);
								controlword = request_controlword(cardrequest);

								log.AppendText("New CW generated: ");
								for(i=0;i<16;i++)
									log.AppendText(Convert.ToString(controlword[i],16).ToUpper() + " ");
								log.AppendText("\r\n");

								soUdp.SendTo(cwserver.generate_cwanswer(usernamecrc,decryptedmessage,controlword, cwserver.kundendb[kundennummer].pwd),remoteEP);
							}
							
						}
						else 
						{
							for(i=0;i<decryptedmessage.Length;i++)
								log.AppendText(Convert.ToString(decryptedmessage[i],16) + " ");
							log.AppendText("\r\n");
						}

					}

				}
					
			}
			catch (SocketException se)
			{
				log.AppendText("A Socket Exception has occurred!" + se.ToString());
			}
				
		}

	}
}
