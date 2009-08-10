sed ' s/ KEY\[/\
KEY[/
s/ FCT\[/\
FCT[/
s/ CMT\[/\
CMT[/
s/ REG\[/\
REG[/
s/ CNR\[/\
CNR[/' $1 | awk '{
gsub("\] +$","]",$0)
if ($0 ~/^F\[/ && $0 !~/\]$/) print $0 "]"
else if ($0 ~/^FCT/ && $0 !~/\]/)
	{
	line = $0
	getline
	gsub("^ +","",$0)
	print line ", " $0
	}
else if ($0 ~/^KEY/ && $0 !~/\]/)
	{
	line = $0
	getline
	gsub("^ +","",$0)
	print line " " $0
	}
else if ($0 ~/^CNR/ && $0 !~/\]/)
	{
	line = $0
	getline
	gsub("^ +","",$0)
	print line " " $0
	}
else print $0}'
