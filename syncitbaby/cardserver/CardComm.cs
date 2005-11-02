using System;


namespace WindowsApplication2
{
	/// <summary>
	/// Zusammenfassung für CardComm.
	/// </summary>
	public class CardComm
	{

		//private Form1 cardcomm = new Form1(1);
		CardSimples cardsimples = new CardSimples();

		public int availableBytes = 0;
		public byte[] indata;
		public byte[] CamKey = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };


		public CardComm()
		{
			;
		}



		public byte[] request_controlword(byte[] cardstring)
		{

			byte[] key1 = new byte[8];
			byte[] key2 = new byte[8];
			byte[] cw   = new byte[16];

			Form1 cardcomm = new Form1();

			CardSimples cardcrypt = new CardSimples();
			//Form1 cardcomm = new Form1();

			cardcomm.com.Output = cardstring;	
			System.Threading.Thread.Sleep(150);

			availableBytes = cardcomm.com.InBufferCount;
			indata = (byte[]) cardcomm.com.Input;

			if((indata[2]==0x9D) && (indata[3]==0x00))
			{
				Array.Copy(indata,14,key1,0,8);
				Array.Copy(indata,22,key2,0,8);

				cardcrypt.ReverseSessionKeyCrypt(cardcomm.CamKey,key1);
				cardcrypt.ReverseSessionKeyCrypt(cardcomm.CamKey,key2);

				Array.Copy(key1,0,cw,0,8);
				Array.Copy(key2,0,cw,8,8);

			}

			return cw;

		}

	}
}
