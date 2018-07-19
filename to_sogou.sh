#/bin/bash

LEAD_IE_LOG=$1
UUID_LOG=$2
LEAD_VALID_LOG=$3

#filter_lead()
{

if [ -s $UUID_LOG ];then
	continue
else
	echo "ERROR: $UUID_LOG doesn't exist 2"
fi


echo "here"

awk -F"\t" '{
	if(FILENAME=="'$UUID_LOG'"){
		uuid[$1]=1;
	} else {
		#if($3=="sogou"){
		{	if($15==0){
			#	print $0 > "'$DISCARD_LOG2'"
			} else if(!uuid[$15]){
				delete uuid[$15]
			#	print $0 > "'$DISCARD_LOG'"
			} else {
				print $0 > "'$LEAD_VALID_LOG'"
			}
		}
	}
}' $UUID_LOG $LEAD_IE_LOG 

}


