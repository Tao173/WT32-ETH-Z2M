//三个主栏字体颜色不为纯黑修改
//ZigStarGW-FW-main/src/websrc/css
//navbar-light .navbar-nav .nav-link{color:rgba(0,0,0,.5)}.
//to
//navbar-light .navbar-nav .nav-link{color:rgb(0,0,0)}.
//
//html.h文件
//硬件信息ETH显示栏
//"<br><strong>&#x786C;&#x4EF6; : </strong>{{hwRev}}"    //硬件信息ETH显示栏
//有线网络配置子栏，主栏下面子信息显示
//"</div><br>"     //有线网络配置子栏，主栏下面子信息显示
//"<div class='card'>"
//"<div class='card-header'>&#x6709;&#x7EBF;&#x7F51;&#x7EDC;</div>"
//"<div class='card-body'>"
//"<div id='ethConfig'>"
//"{{stateEther}}"
//"</div>"
//帮助位置占位
//"<div class='dropdown-menu' aria-labelledby='navbarDropdown'>"
//"<a class='dropdown-item' href='/logs'><i  class='glyphicon glyphicon-transfer'></i>&#x65E5;&#x5FD7;</a>"
//"<a class='dropdown-item' href='/fsbrowser'><i class='glyphicon glyphicon-floppy-disk'></i>&#x6587;&#x4EF6;</a>"
//"<a class='dropdown-item' href='/esp_update'><i class='glyphicon glyphicon-open'></i>&#x5347;&#x7EA7;</a>"
////"<a class='dropdown-item' href='/updates'><i class='glyphicon glyphicon-open'></i>Update Zigbee</a>"          //废弃栏
//"<a class='dropdown-item' href='/reboot'><i class='glyphicon glyphicon-repeat'></i>&#x91CD;&#x542F;</a>"
//"</div>"
//"</li>"
//"<li class='nav-item'>"
//"<a class='nav-link'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</a>"             //帮助位置占位
//"</li>"
//"{{logoutLink}}"
//"</ul></div>"
//"</nav>";
//text-align:center
////navbar-nav{display:-ms-flexbox;display:flex;-ms-flex-direction:column;flex-direction:column;padding-left:0;margin-bottom:0;list-style:none;text-align:center
////navbar-nav{display:-ms-flexbox;display:flex;-ms-flex-direction:column;flex-direction:column;padding-left:0;margin-bottom:0;list-style:none
////.dropdown-menu{position:absolute;top:100%;left:0;z-index:1000;display:none;float:left;min-width:10rem;padding:.5rem 0;margin:.125rem 0 //0;font-size:1rem;color:#212529;text-align:center;