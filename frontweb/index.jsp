<%@ page language="java" pageEncoding="GBK" %>
<%@ page contentType="text/html; charset=GBK" %>
<%@ page import="java.io.*" %>
<%@ page import="javax.servlet.*" %>
<%@ page import="javax.servlet.http.*" %>
<%@ page import="java.util.Date"%> 

<style>
<!--
.b {
        FONT-SIZE: 80%; MARGIN-BOTTOM: 2px; FONT-FAMILY: Arial
}.STYLE1 {
	font-size: x-large;
	font-weight: bold;
	font-family: "Bookman Old Style";
}
-->
</style>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=GBK">
<title>搜索广告查询系统</title>
</head>

<jsp:useBean id="client0" class="adSearch.Client" scope="session" />

<body>

<%   
  Date d = new Date();   
  d.setTime(d.getTime()-24*60*60*1000); 
  int   year = 1900+d.getYear();   
  int   month = 1+d.getMonth();   
  int   date = d.getDate();   
%> 
  
<table border="0" width="100%">
  <tr>
    <td width="50%"><a href="http://www.sogou.com"><img height="60" src="http://www.sogou.com/images/s_logo.gif" width="180" border="0" align="right"></a></td>
    <td width="50%"><span class="STYLE1"><font face="Garamond"><b><font size="5">S</font><font size="4">OHU-</font><font size="5">R</font><font size="4">D</font></b></font> 
      <font face="幼圆" size="5"><b> 搜索广告查询系统</b></font></span></td>             
  </tr>
  <tr>
    <td width="101%" colspan="2">
      <form method="GET" action="results.jsp?" method="post" name="form" onSubmit="return validate_form(document.form.textarea, 2, 5000)" >
        <!--webbot bot="SaveResults" U-File="fpweb:///_private/form_results.txt"
        S-Format="TEXT/CSV" S-Label-Fields="TRUE" -->
        <hr>
<table border="0" align = "center">
    <tr>
      <td width="5%"><b>从</b></td>
      <td width="15%" align="center"><select size="1" name="syearTxt">
          <option <% if (year==2009) out.println("selected");%> >2009</option>
          <option <% if (year==2010) out.println("selected");%> >2010</option>
        </select>年</td>
      <td width="15%" align="center"><select size="1" name="smonthTxt">
          <option <% if (month==1) out.println("selected");%> >01</option>
          <option <% if (month==2) out.println("selected");%> >02</option>
          <option <% if (month==3) out.println("selected");%> >03</option>
          <option <% if (month==4) out.println("selected");%> >04</option>
          <option <% if (month==5) out.println("selected");%> >05</option>
          <option <% if (month==6) out.println("selected");%> >06</option>
          <option <% if (month==7) out.println("selected");%> >07</option>
          <option <% if (month==8) out.println("selected");%> >08</option>
          <option <% if (month==9) out.println("selected");%> >09</option>
          <option <% if (month==10) out.println("selected");%> >10</option>
          <option <% if (month==11) out.println("selected");%> >11</option>
          <option <% if (month==12) out.println("selected");%> >12</option>
        </select>月</td>
      <td width="15%" align="center"><select size="1" name="sdayTxt">
          <option <% if (date==1) out.println("selected");%> >01</option>
          <option <% if (date==2) out.println("selected");%> >02</option>
          <option <% if (date==3) out.println("selected");%> >03</option>
          <option <% if (date==4) out.println("selected");%> >04</option>
          <option <% if (date==5) out.println("selected");%> >05</option>
          <option <% if (date==6) out.println("selected");%> >06</option>
          <option <% if (date==7) out.println("selected");%> >07</option>
          <option <% if (date==8) out.println("selected");%> >08</option>
          <option <% if (date==9) out.println("selected");%> >09</option>
          <option <% if (date==10) out.println("selected");%> >10</option>
          <option <% if (date==11) out.println("selected");%> >11</option>
          <option <% if (date==12) out.println("selected");%> >12</option>
          <option <% if (date==13) out.println("selected");%> >13</option>
          <option <% if (date==14) out.println("selected");%> >14</option>
          <option <% if (date==15) out.println("selected");%> >15</option>
          <option <% if (date==16) out.println("selected");%> >16</option>
          <option <% if (date==17) out.println("selected");%> >17</option>
          <option <% if (date==18) out.println("selected");%> >18</option>
          <option <% if (date==19) out.println("selected");%> >19</option>
          <option <% if (date==20) out.println("selected");%> >20</option>
          <option <% if (date==21) out.println("selected");%> >21</option>
          <option <% if (date==22) out.println("selected");%> >22</option>
          <option <% if (date==23) out.println("selected");%> >23</option>
          <option <% if (date==24) out.println("selected");%> >24</option>
          <option <% if (date==25) out.println("selected");%> >25</option>
          <option <% if (date==26) out.println("selected");%> >26</option>
          <option <% if (date==27) out.println("selected");%> >27</option>
          <option <% if (date==28) out.println("selected");%> >28</option>
          <option <% if (date==29) out.println("selected");%> >29</option>
          <option <% if (date==30) out.println("selected");%> >30</option>
          <option <% if (date==31) out.println("selected");%> >31</option>
        </select>日</td>
    <td width="5%"><b>到</b></td>
      <td width="15%" align="center"><select size="1" name="eyearTxt">
          <option <% if (year==2009) out.println("selected");%> >2009</option>
          <option <% if (year==2010) out.println("selected");%> >2010</option>
        </select>年</td>
      <td width="15%" align="center"><select size="1" name="emonthTxt">
          <option <% if (month==1) out.println("selected");%> >01</option>
          <option <% if (month==2) out.println("selected");%> >02</option>
          <option <% if (month==3) out.println("selected");%> >03</option>
          <option <% if (month==4) out.println("selected");%> >04</option>
          <option <% if (month==5) out.println("selected");%> >05</option>
          <option <% if (month==6) out.println("selected");%> >06</option>
          <option <% if (month==7) out.println("selected");%> >07</option>
          <option <% if (month==8) out.println("selected");%> >08</option>
          <option <% if (month==9) out.println("selected");%> >09</option>
          <option <% if (month==10) out.println("selected");%> >10</option>
          <option <% if (month==11) out.println("selected");%> >11</option>
          <option <% if (month==12) out.println("selected");%> >12</option>
        </select>月</td>
      <td width="15%" align="center"><select size="1" name="edayTxt">
          <option <% if (date==1) out.println("selected");%> >01</option>
          <option <% if (date==2) out.println("selected");%> >02</option>
          <option <% if (date==3) out.println("selected");%> >03</option>
          <option <% if (date==4) out.println("selected");%> >04</option>
          <option <% if (date==5) out.println("selected");%> >05</option>
          <option <% if (date==6) out.println("selected");%> >06</option>
          <option <% if (date==7) out.println("selected");%> >07</option>
          <option <% if (date==8) out.println("selected");%> >08</option>
          <option <% if (date==9) out.println("selected");%> >09</option>
          <option <% if (date==10) out.println("selected");%> >10</option>
          <option <% if (date==11) out.println("selected");%> >11</option>
          <option <% if (date==12) out.println("selected");%> >12</option>
          <option <% if (date==13) out.println("selected");%> >13</option>
          <option <% if (date==14) out.println("selected");%> >14</option>
          <option <% if (date==15) out.println("selected");%> >15</option>
          <option <% if (date==16) out.println("selected");%> >16</option>
          <option <% if (date==17) out.println("selected");%> >17</option>
          <option <% if (date==18) out.println("selected");%> >18</option>
          <option <% if (date==19) out.println("selected");%> >19</option>
          <option <% if (date==20) out.println("selected");%> >20</option>
          <option <% if (date==21) out.println("selected");%> >21</option>
          <option <% if (date==22) out.println("selected");%> >22</option>
          <option <% if (date==23) out.println("selected");%> >23</option>
          <option <% if (date==24) out.println("selected");%> >24</option>
          <option <% if (date==25) out.println("selected");%> >25</option>
          <option <% if (date==26) out.println("selected");%> >26</option>
          <option <% if (date==27) out.println("selected");%> >27</option>
          <option <% if (date==28) out.println("selected");%> >28</option>
          <option <% if (date==29) out.println("selected");%> >29</option>
          <option <% if (date==30) out.println("selected");%> >30</option>
          <option <% if (date==31) out.println("selected");%> >31</option>
        </select>日</td>
    </tr>
</table>
	<hr>
<table border="0" width="100%">
  <tr>
    <td width="10%" rowspan="2" align="center"><b>查询条件</b></td>
    <td width="20%" align="center">查询词 <input type="text" name="QueryTxt" size="13"></td> 
    <td width="20%" align="center">关键字 <input type="text" name="AdkeyTxt" size="13"></td>
    <td width="20%" align="center">广告ID <input type="text" name="AdidTxt" size="13"></td>      
    <td width="16%" align="center">展示区域 <select size="1" name="RgList">       
                <option selected></option>
                <option>小兰条</option>
                <option>左侧</option>
                <option>右侧</option>
              </select></td>
    <td width="14%" align="center">展示位置 <select size="1" name="PosList">          
                <option selected></option>
                <option>1</option>
                <option>2</option>
                <option>3</option>
                <option>4</option>
                <option>5</option>
                <option>6</option>
                <option>7</option>
                <option>8</option>
                <option>9</option>
                <option>10</option>
                <option>11</option>
                <option>12</option>
                <option>13</option>
              </select></td>
  </tr>
  <tr>
    <td width="19%" align="center">客户ID <input type="text" name="AccidTxt" size="13"></td>      
    <td width="19%" align="center">广告来源 <input type="text" name="AdSrcTxt" size="13"></td>      
    <td width="19%" align="center">来源大类 <select size="1" name="AdSrc4List">       
                <option selected></option>
                <option>sohu</option>
                <option>sogou</option>
                <option>sogou-*</option>
                <option>others</option>
              </select></td>
    <td width="18%" align="center" colspan="2">广告类别 <input type="text" name="CataTxt" size="17"></td>      
  </tr>
</table>
        <hr>
<table border="0" width="100%">
  <tr>
    <td width="10%" align="center"><b>展示条件</b></td>
    <td width="6%" align="center"><input type="checkbox" name="QueryChk" value="ON" checked>查询词</td>
    <td width="6%" align="center"><input type="checkbox" name="AdkeyChk" value="ON" checked>关键字</td>
    <td width="6%" align="center"><input type="checkbox" name="AdidChk" value="ON" checked>广告ID</td>
    <td width="6%" align="center"><input type="checkbox" name="RgChk" value="ON" checked>展示区域</td>
    <td width="6%" align="center"><input type="checkbox" name="PosChk" value="ON" checked>展示位置</td>
    <td width="6%" align="center"><input type="checkbox" name="AccidChk" value="ON" checked>客户ID</td>
    <td width="6%" align="center"><input type="checkbox" name="AdsrcChk" value="ON">广告来源</td>
    <td width="6%" align="center"><input type="checkbox" name="Adsrc4Chk" value="ON">来源大类</td>
    <td width="6%" align="center"><input type="checkbox" name="CataChk" value="ON">广告类别</td>
    <td width="7%" align="center">排序按<select size="1" name="sortList">
                <option>PV</option>
                <option selected>Click</option>
                <option>ctr</option>
                <option>cost</option>
      </select></td>
    <td width="5%" align="center">
      <p align="right"><b>滤测试帐号</b></td>
    <td width="2%" align="center">
    <select size="1" name="isTestAccList">
          <option selected>Y</option>
          <option>N</option>
        </select></td>
    <td width="6%" align="center">
      <p align="right"><b>去_free</b></td>
    <td width="2%" align="center">
    <select size="1" name="isPidFreeList">
          <option selected>Y</option>
          <option>N</option>
        </select></td>
    <td width="6%" align="center">
      <p align="right"><b>去cheat</b></td>
    <td width="2%" align="center">
    <select size="1" name="isPidCheat">
          <option>Y</option>
          <option selected>N</option>
        </select></td>
    <td width="6%" align="center">
      <p align="right"><b>最大显示</b></td>
    <td width="2%" align="center">
    <input type="text" name="TopDispTxt" size="3" value=100></td>
  </tr>
</table>   
		<hr>
        <p align="center"><input type="submit" value="查询" name="B1"><input type="reset" value="重写" name="B2"></p>
      </form>
    </td>
  </tr>
</table>

</body>

</html>
