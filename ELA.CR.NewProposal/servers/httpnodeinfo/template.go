package httpnodeinfo

const page = `
<html>
<head>
<meta http-equiv="refresh" content="3">
<title>Node Information</title>
<style type="text/css">
	a:link {color: #FCFCFC}
	a:visited {color: #FCFCFC}
	a:hover {color: #FCFCFC}
	a:active {color: #FCFCFC}
	body {background:#212124; color:#F8F8FF; font-size:20px;}
	td.bh {color: #00FF00}
	td.pk {word-break: break-all; overflow: hidden}
	table.bd {border: 1px solid #111111; font-size:20px;}
	table.bt {border: 1px solid #111111; font-size:25px;}
	table.font {font-size:20px;}
	a.site {cursor:hand; text-decoration:none;}
</style>
</head>

<body>
<center>
<br><br><br>

<table class="bt" width="80%">
	<tr><th>Node Information</th></tr>
</table>
<br>

<table class="bd" width="80%">
<tr>
<td width="20%" >
	<table class="font" width="100%">
	<tr><th>BlockHeight</th></tr>
	<tr><td align="center"><b><font size="40px">{{.BlockHeight}}</font></b></td></tr>
	</table>
</td>
<td width="80%">
	<table class="font" width="100%">
	<tr><td width="25%">NodePort:</td><td width="25%">{{.NodePort}}</td></tr>
	<tr><td width="25%">HttpRestPort:</td><td width="25%">{{.HttpRestPort}}</td><td width="25%">HttpWsPort:</td><td width="25%">{{.HttpWsPort}}</td></tr>
	<tr><td width="25%">HttpJsonPort:</td><td width="25%">{{.HttpJsonPort}}</td></tr>
	</table>
</td>
</tr>
</table>
<br><br><br><br>

<table class="bt" width="80%">
	<tr><th>Neighbors Information</th></tr>
</table>
<br>

<table class="bd" width="80%">
<tr>
<td width="20%" >
	<table class="font" width="100%">
	<tr><th>Neighbor Count</th></tr>
	<tr><td align="center"><b><font size="40px">{{.NeighborCnt}}</font></b></td></tr>
	</table>
</td>
<td width="80%">
	<table class="font" width="100%">
	<tr><th>Neighbor IP</th><th>Neighbor ID</th></tr>
	{{range .Neighbors}}
	<tr><td align="center">{{.NbrAddr}}</td><td align="center"><a href="http://{{.NbrAddr}}/info" style="cursor:hand">{{.NgbID}}</a></td></tr>
	{{end}}
	</table>
</td>
</tr>
</table>
<br><br><br><br><br><br>

<table class="font" border="0" width="80%">
	<tr>
	<td width="26%" align="left"><a href="https://www.elastos.org/blog/" class="site">forum : https://www.elastos.org/blog</a><br></td>
	<td width="26%" align="center"><a href="https://www.elastos.org" class="site">site : https://www.elastos.org</a></td>
	<td width="28%" align="right"><a href="https://www.elastos.org/documents" class="site">doc : https://www.elastos.org/documents</a></td>
	</tr>
</table>
<br><br><br><br>

</center>
</body>
</html>
`
