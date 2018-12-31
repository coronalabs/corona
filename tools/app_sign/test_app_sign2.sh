#!/bin/bash

plaintextInputs=(
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 ticket1@a.com"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 ticket1@a.co"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 ticket1@a.c"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 ticket1@a."
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 ticket1@a"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 ticket1@"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 ticket1"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 ticket"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 ticke"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 tick"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 tic"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 ti"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 t"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051 "
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000051"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 0000005"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 000000"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00000"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 0000"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 000"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 00"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 0"
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01 "
	"53fe3e18f5a81ec8c78223ee9d7b4f9d 4b05f3f5 02 01 01 01"
)

path=`dirname $0`
app_sign=$path/app_sign
if [[ $OSTYPE =~ darwin.* ]]
then
	app_sign=$app_sign.mac
fi

ticket_file=$path/ticket.key

echo "TEST: app_sign encrypt-public/decrypt-private"
echo "      using binary $app_sign"
echo "---------------------------------------------"

iMax=${#plaintextInputs[@]}
for (( i=0; i<iMax; i++))
do
	input=${plaintextInputs[i]}
	#printf "\"%s\"\n" $input
	ticket=`$app_sign encrypt-public $ticket_file "$input"`
	#echo ticket: $ticket
	output=`$app_sign decrypt-private $ticket_file $ticket`

	if [[ "$input" != "$output" ]]
	then
		echo "FAIL:"
		echo "  input:  \"$input\""
		echo "  output: \"$input\""
		echo "  ticket: $ticket"
		echo ""
	else
		echo "PASS: \"$input\""
	fi
#	echo $i
#	echo ${plaintextInputs[i]}
done

# echo $iMax
# for plaintext in ${plaintextInputs}
# do
# 	echo $plaintext
# done
