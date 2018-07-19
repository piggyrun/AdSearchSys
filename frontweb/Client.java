/**
*	�������Ŀͻ���,ʹ�� Tcp ����*
*/
package adSearch;

import java.io.*;
import java.net.*;

public class Client
{
	static final int portNumber = 1983;	
	static final String IP = "10.10.98.137";
	//static final String IP = "10.10.98.179";
	static final int dim = 9;

	public static String connectServer(String request)
	{	 		
		String reply = null;
		try
		{
			Socket server = new Socket(IP, portNumber); // ���� tcp ����
			//server.setSoTimeout(10000); // 3��
			BufferedReader socketIn = new BufferedReader(new InputStreamReader(
                                        server.getInputStream()));
           		PrintWriter socketOut = new PrintWriter(new OutputStreamWriter(
                                        server.getOutputStream(), "GBK"));
			socketOut.print(request);
			socketOut.flush();
			
			System.out.println("send : " + request);
			System.out.println("length : " + request.length());
			
			reply = socketIn.readLine();			
			reply = new String(reply.getBytes("ISO8859_1"),"GBK");
			System.out.println("receive :\nlength : " + reply.length() + "\nresult :" + reply);

			socketIn.close();
			socketOut.close();
			server.close();				
		}
		catch (NullPointerException e)
		{
			System.out.println(e.toString());
		}
		catch (IOException e)
		{
			System.out.println(e.toString());
		}

		String adlist = "";
		if (reply == null)
		{
			adlist += "No response from the server !!!No response from the server !!!";
		}
		else if (reply.equals("no result"))
		{
			adlist += reply + "" + reply;
		}
		else // ��װ��HTML����
		{
			String[] dispName = new String[dim];
			dispName[0] = "��ѯ��";
			dispName[1] = "���ID";
			dispName[2] = "չʾ����";
			dispName[3] = "չʾλ��";
			dispName[4] = "�ͻ�ID";
			dispName[5] = "�����Դ";
			dispName[6] = "��Դ����";
			dispName[7] = "������";
			dispName[8] = "�ؼ���";
			String[] dispUsed = request.split("\t+");
			String dispWord = "";
			String dispWord_forExcel = "";
			int i, j = 0;
			for (i=dim; i<2*dim; i++)
			{
				if (dispUsed[i].equals("1"))
				{
					if (j==0)
					{
						dispWord += dispName[i-dim];
						dispWord_forExcel += dispName[i-dim];
					}
					else
					{
						//dispWord += "+";
						dispWord += "</th><th align=\"center\">";
						dispWord += dispName[i-dim];
						dispWord_forExcel += "</td><td>";
						dispWord_forExcel += dispName[i-dim];
					}
					j++;
				}				
			}
			int disp_num = j;
			//String prefix = "<table align=\"center\" width=\"70%\"><tr><th align=\"center\">չʾ����<br>"+dispWord+"</th><th>PV</th><th>Click</th><th>CTR(%)</th><th>������ã��֣�</th><th>���ۣ��֣�</th></tr>";
			String prefix = "<table align=\"center\" width=\"70%\"><tr><th align=\"center\">"+dispWord+"</th><th>PV</th><th>Click</th><th>CTR(%)</th><th>������ã��֣�</th><th>���ۣ��֣�</th></tr>";
			String data_header = dispWord_forExcel + "\tPV\tClick\tCTR(%)\t������ã��֣�\t���ۣ��֣�\t"; // ����excel��ͷ
			String suffix = "</table>";
			adlist += prefix;
			String[] ads = reply.split("\t+");
			
			int col_num = 6;
			int r = ads.length/col_num;
			int res = ads.length-r*col_num;
			
			for (i = 0; i<r; i++)
			{
				if (i%2 == 0)
					adlist += "<tr bgcolor=\"#C0C0C0\">";
				else
					adlist += "<tr>";
				for (j=0; j<col_num; j++)
				{
					/*
					   if (j==0)
					   adlist += "<td width=\"40%\" align=\"center\">";
					   else
					   adlist += "<td width=\"12%\" align=\"center\">";
					 */
					if (i== r-1 && j==0)
					{
						adlist += ("<td align=\"center\" colspan=\"" + disp_num + "\">");
					}
					else
					{
						adlist += "<td align=\"center\">";
					}
					String content = ads[i*col_num+j];
					if (j==0)
						content = content.replaceAll("</td><td>", "</td><td align=\"center\">");
					adlist += content;
					adlist += "</td>";
				}
				adlist += "</tr>";
			}
			adlist += "</table>";
			if (res != 0)
			{
				adlist += "<b>";
				adlist += ads[col_num*r];
				adlist += "</b>";
			}
			adlist += "<br>";
			adlist += ""+data_header+reply;
		}
		return adlist;
	}
}
