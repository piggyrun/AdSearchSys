/**
*	网络程序的客户端,使用 Tcp 连接*
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
			Socket server = new Socket(IP, portNumber); // 建立 tcp 连接
			//server.setSoTimeout(10000); // 3秒
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
		else // 组装成HTML语言
		{
			String[] dispName = new String[dim];
			dispName[0] = "查询词";
			dispName[1] = "广告ID";
			dispName[2] = "展示区域";
			dispName[3] = "展示位置";
			dispName[4] = "客户ID";
			dispName[5] = "广告来源";
			dispName[6] = "来源大类";
			dispName[7] = "广告分类";
			dispName[8] = "关键字";
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
			//String prefix = "<table align=\"center\" width=\"70%\"><tr><th align=\"center\">展示条件<br>"+dispWord+"</th><th>PV</th><th>Click</th><th>CTR(%)</th><th>点击费用（分）</th><th>均价（分）</th></tr>";
			String prefix = "<table align=\"center\" width=\"70%\"><tr><th align=\"center\">"+dispWord+"</th><th>PV</th><th>Click</th><th>CTR(%)</th><th>点击费用（分）</th><th>均价（分）</th></tr>";
			String data_header = dispWord_forExcel + "\tPV\tClick\tCTR(%)\t点击费用（分）\t均价（分）\t"; // 用于excel表头
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
