BEGIN {
section_label_name = "(^|\t)\\*>([^\\]\\[\t,])+($|\t)"
expansion_list_name="(^|\t)\\*>([^\\]\\[\t,])*\\[([^\\]\\[\t,])+(,([^\\]\\[\t,])+)*\\](\t|$)"
}
{
if ($0 ~ section_label_name) print "Match"
else print "No Match"
}
