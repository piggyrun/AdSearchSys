<%@ page language="java" pageEncoding="GBK" %>
<%@ page contentType="text/html; charset=GBK" %>
<%@ page import="java.io.*" %>
<%@ page import="javax.servlet.*" %>
<%@ page import="javax.servlet.http.*" %>
<%@ page import="java.util.Date"%>
<%@ page import="java.text.SimpleDateFormat"%>
<html>
<style>
<!--
.b {
        FONT-SIZE: 80%; MARGIN-BOTTOM: 2px; FONT-FAMILY: Arial
}.STYLE1 {
	font-size: x-large;
	font-weight: bold;
	font-family: "Bookman Old Style";
}
.STYLE6 {font-weight: bold}
-->
</style>

<head>
<meta http-equiv="Content-Type" content="text/html; charset=GBK">
<title>搜索广告查询系统</title>
</head>

<body>

<jsp:useBean id="client0" class="adSearch.Client" scope="session" />
<!-- jsp:useBean id="count" class="test.GuestCount" scope="application" / -->

<%
	// 来保存文本框内和复选框内的内容，以保持其不刷新
	// 查询条件的保持
	String syear = new String(request.getParameter("syearTxt"));
	String smonth = new String(request.getParameter("smonthTxt"));
	String sday = new String(request.getParameter("sdayTxt"));
	String eyear = new String(request.getParameter("eyearTxt"));
	String emonth = new String(request.getParameter("emonthTxt"));
	String eday = new String(request.getParameter("edayTxt"));
	String Query = new String(request.getParameter("QueryTxt"));
	String Adid = new String(request.getParameter("AdidTxt"));
	String Rg = new String(request.getParameter("RgList"));
	String Pos = new String(request.getParameter("PosList"));
	String Accid = new String(request.getParameter("AccidTxt"));
	String Adsrc = new String(request.getParameter("AdSrcTxt"));
	String Adsrc4 = new String(request.getParameter("AdSrc4List"));
	String Cata = new String(request.getParameter("CataTxt"));
	String Adkey = new String(request.getParameter("AdkeyTxt"));
	
	// 保持展示条件的checkbox	
	String queryCH = "QueryChk\" checked=\"checked\"";
	if (request.getParameter("QueryChk") == null)
		queryCH = "QueryChk";
	
	String adkeyCH = "AdkeyChk\" checked=\"checked\"";
	if (request.getParameter("AdkeyChk") == null)
		adkeyCH = "AdkeyChk";
		
	String AdidCH = "AdidChk";
	if (request.getParameter("AdidChk") != null)
		AdidCH = "AdidChk\" checked=\"checked";
		
	String RgCH = "RgChk";
	if (request.getParameter("RgChk") != null)
		RgCH = "RgChk\" checked=\"checked";
		
	String PosCH = "PosChk";
	if (request.getParameter("PosChk") != null)
		PosCH = "PosChk\" checked=\"checked";
	
	String AccidCH = "AccidChk";
	if (request.getParameter("AccidChk") != null)
		AccidCH = "AccidChk\" checked=\"checked";
	
	String AdsrcCH = "AdsrcChk";
	if (request.getParameter("AdsrcChk") != null)
		AdsrcCH = "AdsrcChk\" checked=\"checked";
		
	String Adsrc4CH = "Adsrc4Chk";
	if (request.getParameter("Adsrc4Chk") != null)
		Adsrc4CH = "Adsrc4Chk\" checked=\"checked";
		
	String CataCH = "CataChk";
	if (request.getParameter("CataChk") != null)
		CataCH = "CataChk\" checked=\"checked";
	
	// 排序依据
	String sort = new String(request.getParameter("sortList"));	
	
	// 保待过滤信息
	String isTestAcc = (String)request.getParameter("isTestAccList");
	String isPidFree = (String)request.getParameter("isPidFreeList");
	String isPidCheat = (String)request.getParameter("isPidCheat");
	String TopDisp = (String)request.getParameter("TopDispTxt");
%>

<table border="0" width="100%">
  <tr>
    <td width="50%"><a href="http://www.sogou.com"><img height="60" src="http://www.sogou.com/images/s_logo.gif" width="180" border="0" align="right"></a></td>
    <td width="50%"><span class="STYLE1"><font face="Garamond"><b><font size="5">S</font><font size="4">OHU-</font><font size="5">R</font><font size="4">D</font></b></font> 
      <font face="幼圆" size="5"><b> 搜索广告查询系统</b></font></span></td>             
  </tr>
  <tr>
    <td width="101%" colspan="2">
      <form method="GET" action="" method="get" name="form" onSubmit="return validate_form(document.form.textarea, 2, 5000)">
        <!--webbot bot="SaveResults" U-File="fpweb:///_private/form_results.txt"
        S-Format="TEXT/CSV" S-Label-Fields="TRUE" -->
        <hr>
<table border="0" align = "center">
    <tr>
      <td width="5%"><b>从</b></td>
      <td width="15%" align="center"><select size="1" name="syearTxt">
          <option <% if (syear.equals("2009")) out.println("selected");%> >2009</option>
          <option <% if (syear.equals("2010")) out.println("selected");%> >2010</option>
        </select>年</td>
      <td width="15%" align="center"><select size="1" name="smonthTxt">
          <option <% if (smonth.equals("01")) out.println("selected");%> >01</option>
          <option <% if (smonth.equals("02")) out.println("selected");%> >02</option>
          <option <% if (smonth.equals("03")) out.println("selected");%> >03</option>
          <option <% if (smonth.equals("04")) out.println("selected");%> >04</option>
          <option <% if (smonth.equals("05")) out.println("selected");%> >05</option>
          <option <% if (smonth.equals("06")) out.println("selected");%> >06</option>
          <option <% if (smonth.equals("07")) out.println("selected");%> >07</option>
          <option <% if (smonth.equals("08")) out.println("selected");%> >08</option>
          <option <% if (smonth.equals("09")) out.println("selected");%> >09</option>
          <option <% if (smonth.equals("10")) out.println("selected");%> >10</option>
          <option <% if (smonth.equals("11")) out.println("selected");%> >11</option>
          <option <% if (smonth.equals("12")) out.println("selected");%> >12</option>
        </select>月</td>
      <td width="15%" align="center"><select size="1" name="sdayTxt">
          <option <% if (sday.equals("01")) out.println("selected");%> >01</option>
          <option <% if (sday.equals("02")) out.println("selected");%> >02</option>
          <option <% if (sday.equals("03")) out.println("selected");%> >03</option>
          <option <% if (sday.equals("04")) out.println("selected");%> >04</option>
          <option <% if (sday.equals("05")) out.println("selected");%> >05</option>
          <option <% if (sday.equals("06")) out.println("selected");%> >06</option>
          <option <% if (sday.equals("07")) out.println("selected");%> >07</option>
          <option <% if (sday.equals("08")) out.println("selected");%> >08</option>
          <option <% if (sday.equals("09")) out.println("selected");%> >09</option>
          <option <% if (sday.equals("10")) out.println("selected");%> >10</option>
          <option <% if (sday.equals("11")) out.println("selected");%> >11</option>
          <option <% if (sday.equals("12")) out.println("selected");%> >12</option>
          <option <% if (sday.equals("13")) out.println("selected");%> >13</option>
          <option <% if (sday.equals("14")) out.println("selected");%> >14</option>
          <option <% if (sday.equals("15")) out.println("selected");%> >15</option>
          <option <% if (sday.equals("16")) out.println("selected");%> >16</option>
          <option <% if (sday.equals("17")) out.println("selected");%> >17</option>
          <option <% if (sday.equals("18")) out.println("selected");%> >18</option>
          <option <% if (sday.equals("19")) out.println("selected");%> >19</option>
          <option <% if (sday.equals("20")) out.println("selected");%> >20</option>
          <option <% if (sday.equals("21")) out.println("selected");%> >21</option>
          <option <% if (sday.equals("22")) out.println("selected");%> >22</option>
          <option <% if (sday.equals("23")) out.println("selected");%> >23</option>
          <option <% if (sday.equals("24")) out.println("selected");%> >24</option>
          <option <% if (sday.equals("25")) out.println("selected");%> >25</option>
          <option <% if (sday.equals("26")) out.println("selected");%> >26</option>
          <option <% if (sday.equals("27")) out.println("selected");%> >27</option>
          <option <% if (sday.equals("28")) out.println("selected");%> >28</option>
          <option <% if (sday.equals("29")) out.println("selected");%> >29</option>
          <option <% if (sday.equals("30")) out.println("selected");%> >30</option>
          <option <% if (sday.equals("31")) out.println("selected");%> >31</option>
        </select>日</td>
    <td width="5%"><b>到</b></td>
      <td width="15%" align="center"><select size="1" name="eyearTxt">
          <option <% if (eyear.equals("2009")) out.println("selected");%> >2009</option>
          <option <% if (eyear.equals("2010")) out.println("selected");%> >2010</option>
        </select>年</td>
      <td width="15%" align="center"><select size="1" name="emonthTxt">
          <option <% if (emonth.equals("01")) out.println("selected");%> >01</option>
          <option <% if (emonth.equals("02")) out.println("selected");%> >02</option>
          <option <% if (emonth.equals("03")) out.println("selected");%> >03</option>
          <option <% if (emonth.equals("04")) out.println("selected");%> >04</option>
          <option <% if (emonth.equals("05")) out.println("selected");%> >05</option>
          <option <% if (emonth.equals("06")) out.println("selected");%> >06</option>
          <option <% if (emonth.equals("07")) out.println("selected");%> >07</option>
          <option <% if (emonth.equals("08")) out.println("selected");%> >08</option>
          <option <% if (emonth.equals("09")) out.println("selected");%> >09</option>
          <option <% if (emonth.equals("10")) out.println("selected");%> >10</option>
          <option <% if (emonth.equals("11")) out.println("selected");%> >11</option>
          <option <% if (emonth.equals("12")) out.println("selected");%> >12</option>
        </select>月</td>
      <td width="15%" align="center"><select size="1" name="edayTxt">
          <option <% if (eday.equals("01")) out.println("selected");%> >01</option>
          <option <% if (eday.equals("02")) out.println("selected");%> >02</option>
          <option <% if (eday.equals("03")) out.println("selected");%> >03</option>
          <option <% if (eday.equals("04")) out.println("selected");%> >04</option>
          <option <% if (eday.equals("05")) out.println("selected");%> >05</option>
          <option <% if (eday.equals("06")) out.println("selected");%> >06</option>
          <option <% if (eday.equals("07")) out.println("selected");%> >07</option>
          <option <% if (eday.equals("08")) out.println("selected");%> >08</option>
          <option <% if (eday.equals("09")) out.println("selected");%> >09</option>
          <option <% if (eday.equals("10")) out.println("selected");%> >10</option>
          <option <% if (eday.equals("11")) out.println("selected");%> >11</option>
          <option <% if (eday.equals("12")) out.println("selected");%> >12</option>
          <option <% if (eday.equals("13")) out.println("selected");%> >13</option>
          <option <% if (eday.equals("14")) out.println("selected");%> >14</option>
          <option <% if (eday.equals("15")) out.println("selected");%> >15</option>
          <option <% if (eday.equals("16")) out.println("selected");%> >16</option>
          <option <% if (eday.equals("17")) out.println("selected");%> >17</option>
          <option <% if (eday.equals("18")) out.println("selected");%> >18</option>
          <option <% if (eday.equals("19")) out.println("selected");%> >19</option>
          <option <% if (eday.equals("20")) out.println("selected");%> >20</option>
          <option <% if (eday.equals("21")) out.println("selected");%> >21</option>
          <option <% if (eday.equals("22")) out.println("selected");%> >22</option>
          <option <% if (eday.equals("23")) out.println("selected");%> >23</option>
          <option <% if (eday.equals("24")) out.println("selected");%> >24</option>
          <option <% if (eday.equals("25")) out.println("selected");%> >25</option>
          <option <% if (eday.equals("26")) out.println("selected");%> >26</option>
          <option <% if (eday.equals("27")) out.println("selected");%> >27</option>
          <option <% if (eday.equals("28")) out.println("selected");%> >28</option>
          <option <% if (eday.equals("29")) out.println("selected");%> >29</option>
          <option <% if (eday.equals("30")) out.println("selected");%> >30</option>
          <option <% if (eday.equals("31")) out.println("selected");%> >31</option>
        </select>日</td>
    </tr>
</table>
        <hr>
<table border="0" width="100%">
  <tr>
    <td width="10%" rowspan="2" align="center"><b>查询条件</b></td>
    <td width="20%" align="center">查询词 <input type="text" name="QueryTxt" size="13" value="<%=Query%>"></td> 
    <td width="20%" align="center">关键字 <input type="text" name="AdkeyTxt" size="13" value="<%=Adkey%>"></td>
    <td width="20%" align="center">广告ID <input type="text" name="AdidTxt" size="13" value="<%=Adid%>"></td>      
    <td width="16%" align="center">展示区域 <select size="1" name="RgList">       
                <option <% if (Rg.equals("")) out.println("selected");%> ></option>
                <option <% if (Rg.equals("小兰条")) out.println("selected");%> >小兰条</option>
                <option <% if (Rg.equals("左侧")) out.println("selected");%> >左侧</option>
                <option <% if (Rg.equals("右侧")) out.println("selected");%> >右侧</option>
              </select></td>
    <td width="14%" align="center">展示位置 <select size="1" name="PosList">          
                <option <% if (Pos.equals("")) out.println("selected");%> ></option>
                <option <% if (Pos.equals("1")) out.println("selected");%> >1</option>
                <option <% if (Pos.equals("2")) out.println("selected");%> >2</option>
                <option <% if (Pos.equals("3")) out.println("selected");%> >3</option>
                <option <% if (Pos.equals("4")) out.println("selected");%> >4</option>
                <option <% if (Pos.equals("5")) out.println("selected");%> >5</option>
                <option <% if (Pos.equals("6")) out.println("selected");%> >6</option>
                <option <% if (Pos.equals("7")) out.println("selected");%> >7</option>
                <option <% if (Pos.equals("8")) out.println("selected");%> >8</option>
                <option <% if (Pos.equals("9")) out.println("selected");%> >9</option>
                <option <% if (Pos.equals("10")) out.println("selected");%> >10</option>
                <option <% if (Pos.equals("11")) out.println("selected");%> >11</option>
                <option <% if (Pos.equals("12")) out.println("selected");%> >12</option>
                <option <% if (Pos.equals("13")) out.println("selected");%> >13</option>
              </select></td>
  </tr>
  <tr>
    <td width="19%" align="center">客户ID <input type="text" name="AccidTxt" size="13" value="<%=Accid%>"></td>      
    <td width="19%" align="center">广告来源 <input type="text" name="AdSrcTxt" size="13" value="<%=Adsrc%>"></td>      
    <td width="19%" align="center">来源大类 <select size="1" name="AdSrc4List">       
                <option <% if (Adsrc4.equals("")) out.println("selected");%> ></option>
                <option <% if (Adsrc4.equals("sohu")) out.println("selected");%> >sohu</option>
                <option <% if (Adsrc4.equals("sogou")) out.println("selected");%> >sogou</option>
                <option <% if (Adsrc4.equals("sogou-*")) out.println("selected");%> >sogou-*</option>
                <option <% if (Adsrc4.equals("others")) out.println("selected");%> >others</option>
              </select></td>
    <td width="18%" align="center"  colspan="2">广告类别 <input type="text" name="CataTxt" size="17" value="<%=Cata%>"></td>      
  </tr>
</table>
        <hr>
<table border="0" width="100%">
  <tr>
    <td width="10%" align="center"><b>展示条件</b></td>
    <td width="6%" align="center"><input type="checkbox" name="<%=queryCH%>" value="ON">查询词</td>
    <td width="6%" align="center"><input type="checkbox" name="<%=adkeyCH%>" value="ON">关键字</td>
    <td width="6%" align="center"><input type="checkbox" name="<%=AdidCH%>" value="ON">广告ID</td>
    <td width="6%" align="center"><input type="checkbox" name="<%=RgCH%>" value="ON">展示区域</td>
    <td width="6%" align="center"><input type="checkbox" name="<%=PosCH%>" value="ON">展示位置</td>
    <td width="6%" align="center"><input type="checkbox" name="<%=AccidCH%>" value="ON">客户ID</td>
    <td width="6%" align="center"><input type="checkbox" name="<%=AdsrcCH%>" value="ON">广告来源</td>
    <td width="6%" align="center"><input type="checkbox" name="<%=Adsrc4CH%>" value="ON">来源大类</td>
    <td width="6%" align="center"><input type="checkbox" name="<%=CataCH%>" value="ON">广告类别</td>
    <td width="7%" align="center">排序按<select size="1" name="sortList">
                <option <% if (sort.equals("PV")) out.println("selected");%> >PV</option>
                <option <% if (sort.equals("Click")) out.println("selected");%> >Click</option>
                <option <% if (sort.equals("ctr")) out.println("selected");%> >ctr</option>
                <option <% if (sort.equals("cost")) out.println("selected");%> >cost</option>
      </select></td>
    <td width="5%" align="center">
      <p align="right"><b>滤测试帐号</b></td>
    <td width="2%" align="center">
    <select size="1" name="isTestAccList">
          <option <% if (isTestAcc.equals("Y")) out.println("selected");%> >Y</option>
          <option <% if (isTestAcc.equals("N")) out.println("selected");%> >N</option>
        </select></td>
    <td width="6%" align="center">
      <p align="right"><b>去_free</b></td>
    <td width="2%" align="center">
    <select size="1" name="isPidFreeList">
          <option <% if (isPidFree.equals("Y")) out.println("selected");%> >Y</option>
          <option <% if (isPidFree.equals("N")) out.println("selected");%> >N</option>
        </select></td>
    <td width="6%" align="center">
      <p align="right"><b>去cheat</b></td>
    <td width="2%" align="center">
    <select size="1" name="isPidCheat">
          <option <% if (isPidCheat.equals("Y")) out.println("selected");%> >Y</option>
          <option <% if (isPidCheat.equals("N")) out.println("selected");%> >N</option>
        </select></td>
    <td width="6%" align="center">
      <p align="right"><b>最大显示</b></td>
    <td width="2%" align="center">
    <input type="text" name="TopDispTxt" size="3" value="<%=TopDisp%>"></td>
  </tr>
</table>          
		<hr>
        <p align="center"><input type="submit" value="查询" name="B1"><input type="reset" value="重写" name="B2"></p>
      </form>
    </td>
  </tr>
</table>

<%	 
	//request.setCharacterEncoding("GBK");
	//下面的if语句将判断检查框是否输出值
	//若有输出值，则代表该检查框被选取
	//String requestValue = (String)request.getParameter("textarea");
	//requestValue = requestValue.replaceAll("\r\n", " ");	//处理换行符,因为服务器读取按行
	//requestValue = new String(requestValue.getBytes(), "GBK");

	String param = "";
	// 格式：查询（展示）条件+\t（各九个个，共十八，如果条件未选，则为""+排序依据+过滤测试，最后是起始日期(格式20090101)+\t+结束日期+当天日期
	// 查询
	
	if (Query.equals(""))
		param += "!";
	else
		param += Query;
	param += "\t";
	if (Adid.equals(""))
		param += "!";
	else
		param += Adid;
	param += "\t";
	if (Rg.equals(""))
		param += "!";
	else
		param += Rg;
	param += "\t";
	if (Pos.equals(""))
		param += "!";
	else
		param += Pos;
	param += "\t";
	if (Accid.equals(""))
		param += "!";
	else
		param += Accid;
	param += "\t";
	if (Adsrc.equals(""))
		param += "!";
	else
		param += Adsrc;
	param += "\t";
	if (Adsrc4.equals(""))
		param += "!";
	else
		param += Adsrc4;
	param += "\t";
	if (Cata.equals(""))
		param += "!";
	else
		param += Cata;
	param += "\t";
	if (Adkey.equals(""))
		param += "!";
	else
		param += Adkey;
	param += "\t";
	// 展示
	if (request.getParameter("QueryChk") != null)
		param += "1\t";
	else
		param += "0\t";
	if (request.getParameter("AdidChk") != null)
		param += "1\t";
	else
		param += "0\t";
	if (request.getParameter("RgChk") != null)
		param += "1\t";
	else
		param += "0\t";
	if (request.getParameter("PosChk") != null)
		param += "1\t";
	else
		param += "0\t";
	if (request.getParameter("AccidChk") != null)
		param += "1\t";
	else
		param += "0\t";
	if (request.getParameter("AdsrcChk") != null)
		param += "1\t";
	else
		param += "0\t";
	if (request.getParameter("Adsrc4Chk") != null)
		param += "1\t";
	else
		param += "0\t";
	if (request.getParameter("CataChk") != null)
		param += "1\t";
	else
		param += "0\t";
	if (request.getParameter("AdkeyChk") != null)
		param += "1\t";
	else
		param += "0\t";
	
	// sort依据
	if (sort.equals("PV"))
		param += "0\t";
	else if (sort.equals("Click"))
		param += "1\t";
	else if (sort.equals("ctr"))
		param += "2\t";
	else
		param += "3\t";
	
	// 过滤测试帐号
	if (isTestAcc.equals("Y"))//过滤
		param += "1\t";
	else //不过滤
		param += "0\t";
	// 过滤sohu__free sogou__free
	if (isPidFree.equals("Y"))//过滤
		param += "1\t";
	else //不过滤
		param += "0\t";
	// 过滤cheatpid
	if (isPidCheat.equals("Y"))//过滤
		param += "1\t";
	else //不过滤
		param += "0\t";
	// 最大显示行数
	param += TopDisp;
	param += "\t";
		
	// 日期
	SimpleDateFormat formatter = new SimpleDateFormat("yyyyMMdd"); 
	Date currentTime = new Date(); 
	String dateString= formatter.format(currentTime);
 
	param += syear+smonth+sday+"\t"+eyear+emonth+eday+"\t"+dateString;
	
	String reply_buf = client0.connectServer(param);	// 传送给查询服务器请求，然后接受请求
	String[] data = reply_buf.split(""); // 前面是html化的数据，后面是原数据
  
	session.setAttribute("ret_res", data[1]); // 保存返回数据
	out.println("<table border=\"0\" align=\"center\">");
	out.println("<tr><td>");
	out.print("<div align=\"center\"><a href=\"export.jsp\">&gt;&gt;导出Excel文件&lt;&lt;</a></div>");
	out.println("</td></tr></table>");

	out.println(data[0] + "<br>");
  	
	/*if (requestValue.length() < 30)
		out.println("查询串: " + requestValue + "");
	out.println("<br><br><strong>结果:</strong><br>");
	if (reply_buf == null)
		out.println("<center>" + "error of server!!!:)" + "</center><br> <br>");
	else
		out.println(reply_buf + "<br>");//*/
%>


</body>

</html>
