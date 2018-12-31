#!/bin/bash

# builds output.zip for iPhone

# $1 = device id hashed as md5, e.g. md5(MAC ethernet id)
# $2 = expiration date (UNIX time)
# $3 = subscription type (enum)
# $4 = license type (enum)
# $5 = platform (enum)
# $6 = subscription state (0 = expired; 1 = active)
# $7 = uid (number)
# $8 = username (string)
# $9 = key 

# Example:
# ./create_ticket.sh 001ec20b35bb `date -u -v +31d "+%s"` 1 1 1 5 e3638f4082b08abc04a42c7b75ade1d3 web-a1.ath.cx ticket.key
# where the expiration date (date -u -v +30d "+%s") is 30 days from today (we do 31 to account for time zone differences)

path=`dirname $0`

#deviceHash=`echo -n $1 | md5`
#deviceHash=`echo -n $1 | openssl dgst -md5`

#	#deviceHash=`printf %s $deviceHash`
#	expiration=`printf "%08x" $2`
#	subType=`printf "%02x" $3`
#	licenseType=`printf "%02x" $4`
#	platformType=`printf "%02x" $5`
#	reserved="00"
#	uid=`printf "%08x" $6`
#
#	echo $deviceHash $expiration $subType $licenseType $platformType $reserved $uid $7 $8
#printf -v ticket "%s %08x %02x %02x %02x %02x %08x %s %s" $deviceHash $2 $3 $4 $5 "00" $6 $7 $8
#printf "%s %08x %02x %02x %02x %02x %08x %s %s" $deviceHash $2 $3 $4 $5 "00" $6 $7 $8
#printf "%s %08x %02x %02x %02x %02x %08x %s %s\n" $deviceHash $2 $3 $4 $5 "00" $6 $7 $8
#ticket=`echo -n $(printf "%s %08x %02x %02x %02x %02x %08x %s %s" $deviceHash $2 $3 $4 $5 "00" $6 $7 $8)`

ticket=`printf %s "$(printf "%s %08x %02x %02x %02x %02x %08x %s" $1 $2 $3 $4 $5 $6 $7 $8)"`
#echo "$ticket"

if [[ -e $9 ]]
then
	"$path/../bin/mac/app_sign" encrypt $9 "$ticket"
else
	#printf $ticket
	#ticket=`echo -n $(printf "%s %08x %02x %02x %02x %02x %08x %s" $1 $2 $3 $4 $5 $6 $7 $8)`
	printf %s "$ticket"
fi

