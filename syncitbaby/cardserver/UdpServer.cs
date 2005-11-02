using System;
using System.Net;
using System.Net.Sockets;

namespace WindowsApplication2
{
	/// <summary>
	/// Zusammenfassung für UdpServer.
	/// </summary>
	public class UdpServer
	{
		private const int sampleUdpPort = 4568;

		public UdpServer()
		{
			//
			// TODO: Fügen Sie hier die Konstruktorlogik hinzu
			//
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

					Array.Copy(received,0,usernamecrc,0,4);
					Array.Copy(received,4,cryptedmessage,0,112);


					if(cwserver.auth_client(usernamecrc)==1)
					{
						decryptedmessage = cwserver.decrypt_message(cryptedmessage, "gangbang", 112);

						if(cwserver.validate_request(decryptedmessage)==1)
						{
							cardrequest = cwserver.generate_cardrequest(decryptedmessage);
							Form1 test = new Form1();
							controlword = cardcomm.request_controlword(cardrequest);
							//test.com.Output = cardrequest;
						}
					}

					if(bytesReceived!=0)
					{
						for(i=0;i<bytesReceived;i++)
							; //log.AppendText(Convert.ToString(received[i],16) + " ");
					}
				
					//String returningString = "The Server got your message through UDP:" + dataReceived;
						
					//Byte[] returningByte = System.Text.Encoding.ASCII.GetBytes(returningString.ToCharArray());
						
					//soUdp.SendTo(returningByte, remoteEP);


				}
					
			}
			catch (SocketException se)
			{
				return ; //udpdebug.AppendText("A Socket Exception has occurred!" + se.ToString());
			}
				
		}
	}
}
