 REM By Krb
 screen 2,2
 color 5,5
 cls

 cursor 0,8
 color 1,7
 gosub dologo

 cursor 6,9
 color 15,7
 gosub dologo

 cursor 0,72
 color 7,5
 gosub domoto

endl
 goto endl

dologo:
 print""
 print"  ▁▁▁▁▁.▁.▁  .▁      .▁         ▁▁▁▁   "
 print" ╱  ▁▁╱│▁│ │ │▁│ ▁▁▁ │▁│▁▁ ▁▁  ╱    ╲  "
 print" ╲▁▁ ╲ │ │ │ │ │╱ ▁▁╲│ │  │  ╲╱ ╲ ╱  ╲ "
 print" ╱    ╲│ │ │▁│ ╲  ╲▁▁│ │  │  ╱   Y    ╲"
 print"╱▁▁▁  ╱│▁│▁▁▁╱▁│╲▁▁  >▁│▁▁▁▁╱╲▁▁▁│▁▁  ╱"
 print"    ╲╱             ╲╱               ╲╱ "
 return

domoto:
 print"  Defends ton R.E.T.R.O.F.U.T.U.R. au  "
 return

 print"___________     ________      _________"
 print"\__    ___/    /  _____/     /   _____/"
 print"  |    |      /   \  ___     \_____  \ "
 print"  |    |      \    \_\  \    /        \"
 print"  |____|       \______  /   /_______  /"
 print" Toulouse        Game \/       Show \/ "

 return


