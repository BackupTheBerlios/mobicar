using System;
using System.Collections;
using System.IO;
using System.Data;
using System.Text;
using System.Security.Cryptography;


namespace WindowsApplication2
{
	/// <summary>
	/// Zusammenfassung für CWServer.
	/// </summary>
	public class CWServer
	{

		public struct account
		{
			public string usr;
			public string pwd;
			public int uniq;
			public int abo;
		};

		public account[] kundendb = new account[4];

		
		public struct abo
		{
			public string name;
			public int[] ChIDs;
			public int price;
		}

		public abo[] abonnamentdb = new abo[11];


		
		public CWServer()
		{
			kundendb[0].usr = "root1";
			kundendb[0].pwd = "gangbang";

			kundendb[1].usr = "root2";
			kundendb[1].pwd = "gangbang";
			
			kundendb[2].usr = "root3";
			kundendb[2].pwd = "gangbang";
			
			kundendb[3].usr = "root";
			kundendb[3].pwd = "gangbang";
			
			abonnamentdb[0].name = "Premiere Start";
			abonnamentdb[0].ChIDs = new int[] { 0x0008 };
			abonnamentdb[0].price = 0;

			abonnamentdb[1].name = "Premiere Thema";
			abonnamentdb[1].ChIDs = new int[] { 0x0017, 0x0010, 0x0204, 0x0203, 0x001C, 0x0013, 0x000E, 0x000C, 0x000D, 0x00A8, 0x0018, 0x002A, 0x000F, 0x0024 };
			abonnamentdb[1].price = 0;

			abonnamentdb[2].name = "Premiere Sport";
			abonnamentdb[2].ChIDs = new int[] { 0x0000 };
			abonnamentdb[2].price = 0;

			abonnamentdb[3].name = "Premiere Film";
			abonnamentdb[3].ChIDs = new int[] { 0x0008, 0x000A, 0x000B, 0x002B, 0x0009, 0x001D, 0x0029, 0x0014, 0x0013, 0x0022 };
			abonnamentdb[3].price = 0;

			abonnamentdb[4].name = "Premiere Sport + Thema";
			abonnamentdb[4].ChIDs = new int[] { 0x0017, 0x0010, 0x0204, 0x0203, 0x001C, 0x0013, 0x000E, 0x000C, 0x000D, 0x00A8, 0x0018, 0x002A, 0x000F, 0x0024 };
			abonnamentdb[4].price = 0;

		}

		public int auth_client(byte[] givenusername)
		{

			int kdnr = 0;

			for(kdnr=0;kdnr<kundendb.Length;kdnr++)
			{

				MD5 uname = new MD5CryptoServiceProvider();
				ASCIIEncoding uname_md5_encoding=new ASCIIEncoding();
				byte[] uname_md5 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
				uname_md5 = uname.ComputeHash(uname_md5_encoding.GetBytes(kundendb[kdnr].usr));
				uname.Clear();

				//ASCIIEncoding crc32_encoding = new ASCIIEncoding();
				CRC32 crc = new CRC32(0xedb88320); // equivalent to new CRC32(CRC32.DefaultPolynomial);
				byte[] uname_crc32 = { 0, 0, 0, 0 };
				uname_crc32 = crc.ComputeHash(uname_md5);
				crc.Clear();

				if((givenusername[0]==uname_crc32[0]) && (givenusername[1]==uname_crc32[1]) &&(givenusername[2]==uname_crc32[2]) &&(givenusername[3]==uname_crc32[3]))
					{
						return kdnr;
					}
				
			}

			return -1; //lets return the "kundennummer" of the right viewer
		}


		public byte[] decrypt_message(byte[] message, string key, int length)
		{
			MemoryStream ms = new MemoryStream(); 
			byte[] messagecopy = new byte[128];
			Array.Copy(message,0,messagecopy,0,length);

			MD5 passwd = new MD5CryptoServiceProvider();
			ASCIIEncoding passwd_md5_encoding=new ASCIIEncoding();
			byte[] passwd_md5 = passwd.ComputeHash(passwd_md5_encoding.GetBytes(key));
		

			RijndaelManaged aes = new RijndaelManaged();
			aes.Mode = CipherMode.ECB;
			aes.BlockSize = 128;
			aes.KeySize = 128; //128-Bit AES En/Decryption
			aes.Key = passwd_md5;

			CryptoStream cs = new CryptoStream(ms, aes.CreateDecryptor(), CryptoStreamMode.Write); 
			cs.Write(messagecopy, 0, length+16); 
			aes.Clear();

			byte[] decryptedData = ms.ToArray(); 

			return decryptedData;

		}


		public int validate_request(byte[] message)
		{
			int i=0;

			switch(message[0])
			{
				case 0:
				{
					byte length = message[1];
					byte[] datacrc = new byte[4];
					byte[] data = new byte[length];

					Array.Copy(message,4,datacrc,0,4);
					Array.Copy(message,20,data,0,length);

					CRC32 crc = new CRC32(0xedb88320);
					byte[] data_crc32 = crc.ComputeHash(data);

					for(i=0;i<4;i++)
					{
						if(datacrc[i]!=data_crc32[i])
						{
							return -1; //crc not correct - data broken !
						}
					}

					return 1; // data is ok, we can proceed ...
				}
				case 2:
				{
					return 0; //not implemented yet
				}
			}
			return -1;
		}



		public int validate_abo(byte[] message, int abonr)
		{
			int valid_abonnament = -1;
			int i = 0;

			int ChID = 0x0000;

			ChID = (message[8]<<8) + message[9];

			for(i=0;i<abonnamentdb[abonr].ChIDs.Length;i++)
			{
				if(abonnamentdb[abonr].ChIDs[i]==ChID)
				{
					valid_abonnament = 1;
				}
			}

			return valid_abonnament;
		}


		public byte[] generate_cardrequest(byte[] message)
		{
			/* |  |  |     |           |     |     |           |     |     |
			 *  00 54 C4 2C 0F F1 BA 6F 00 14 17 22 00 00 00 00 F0 14 DD 40
			 * first byte is mode: 00=> ECM, 02=>EMM
			 * second byte is lengh of data (ECM,EMM)
			 * third and fourth byte are fillers
			 * next 4 bytes are crc-checksum of data (ECM,EMM)
			 * next 2 bytes is Channel-ID (important for later "Abos")
			 * then comes Provider-ID (Cable or Sattelite)
			 * some 4 Byte crap (always zero) ...
			 */

			int ChID = 0x0000;
			byte[] PID = new byte[2];
			byte[] ECM = new byte[85];
			byte[] modECM = { 0x01,0x05,0x00,0x00,0x02,0x4E };

			
			ChID = (message[8]<<8) + message[9];
			Array.Copy(message,10,PID,0,2);
			Array.Copy(message,20,ECM,0,84);
			Array.Copy(modECM,0,ECM,0,6);

			CardSimples xorbyte = new CardSimples();

			ECM[84] = xorbyte.XorSum(ECM,84);
			return ECM;
		}

		public byte[] generate_cwanswer(byte[] checksum, byte[] message, byte[] cw, string key)
		{
			byte[] cwanswer = new byte[48];
			byte[] cryptedanswer = new byte[52];
			MemoryStream ms = new MemoryStream(); 

			CRC32 crc = new CRC32(0xedb88320); // equivalent to new CRC32(CRC32.DefaultPolynomial);
			byte[] cw_crc32 = crc.ComputeHash(cw);
			
			//Array.Copy(checksum,0,cwanswer,0,4);

			Array.Copy(message,0,cwanswer,0,20);
			Array.Copy(cw_crc32,0,cwanswer,4,4);
			Array.Copy(cw,0,cwanswer,20,16);
			cwanswer[0] = 0x01;
			cwanswer[1] = 0x10;			
			cwanswer[2] = 0x32;
			cwanswer[3] = 0x84;


			for(int i=0;i<12;i++)
				cwanswer[i+36] = 0xFF;

			MD5 passwd = new MD5CryptoServiceProvider();
			ASCIIEncoding passwd_md5_encoding=new ASCIIEncoding();
			byte[] passwd_md5 = passwd.ComputeHash(passwd_md5_encoding.GetBytes(key));
		

			RijndaelManaged aes = new RijndaelManaged();
			aes.Mode = CipherMode.ECB;
			aes.BlockSize = 128;
			aes.KeySize = 128; //128-Bit AES En/Decryption
			aes.Key = passwd_md5;

			CryptoStream cs = new CryptoStream(ms, aes.CreateEncryptor(), CryptoStreamMode.Write); 
			cs.Write(cwanswer, 0, 48); 
			aes.Clear();

			byte[] cryptedData = ms.ToArray(); 

			Array.Copy(cryptedData,0,cryptedanswer,4,48);
			Array.Copy(checksum,0,cryptedanswer,0,4);

			return cryptedanswer;
		}

	}
}
