<%@ page language="java" contentType="text/html;charset=gb2312"%>
<%@ page import="java.io.*" %>
<%@ page import="javax.servlet.*" %>
<%@ page import="javax.servlet.http.*" %>
<%@ page import="org.apache.poi.hssf.usermodel.HSSFRow"%>
<%@ page import="org.apache.poi.hssf.usermodel.HSSFSheet"%>
<%@ page import="org.apache.poi.hssf.usermodel.HSSFWorkbook"%>
<style>
<!--
span.errmsg{
	    color: #ff0000;
}
-->
</style>
<%
String filename="export";
%>


<%
HSSFWorkbook wb = new HSSFWorkbook();
HSSFSheet sheet = wb.createSheet("sheet1");
HSSFRow row;

String data = (String)session.getAttribute("ret_res");
String[] ads = data.split("\t+");

int col_num = 6;
int r = ads.length/col_num;
int res = ads.length-r*col_num;

int i,j,ll;
int header_num = 1;
for (i = 0; i<r; i++)
{
	row = sheet.createRow(i);
	if (i<r-1)
	{
		for (j=0; j<col_num; j++)
		{
			if (j==0)
			{
				String[] header = ads[i*col_num+j].split("</td><td>");
				header_num = header.length;
				for (ll = 0; ll<header_num; ll++)
				{
					String content = header[ll];
					row.createCell(ll).setCellValue(content);
				}
			}
			else
			{
				String content = ads[i*col_num+j];
				row.createCell(j+header_num-1).setCellValue(content);
			}
		}
	}
	else // 最后一行header只有一个
	{
		for (j=0; j<col_num; j++)
		{
			// 补空列
			for (ll = 0; ll < header_num-1; ll++)
			{
				row.createCell(ll).setCellValue("");
			}
			String content = ads[i*col_num+j];
			row.createCell(j+header_num-1).setCellValue(content);
		}
	}
}
if (res != 0)
{
	row = sheet.createRow(r);
	String content = ads[col_num*r];
	row.createCell(0).setCellValue(content);
}

response.reset();
response.setContentType("application/ms-excel");
response.setCharacterEncoding("GB2312");
response.addHeader("Content-Disposition", "filename=" + filename + ".xls");
wb.write(response.getOutputStream());
response.getOutputStream().flush();
response.getOutputStream().close();
%>
