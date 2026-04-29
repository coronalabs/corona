#!/bin/bash

# Prints test tickets
# $1 is UID of the user in Drupal
# $2 is the corresponding e-mail address
# $3 is an optional path to a ticket file.  If it exists, then the encrypted ticket
#       is printed instead of plain text

path=`dirname $0`
macId=`ifconfig en0 ether | grep ether | sed "s/[^0-9,a-f]//;s/[ :]//g;s/ether//"`
deviceId=`echo -n $macId | md5`

active=`date -u -v +30d "+%s"`
activeAlmostExpired=`date -u -v +5d "+%s"`
activeOneDayLeft=`date -u -v +1d "+%s"`
activeLessThanOneDayLeft=`date -u -v +20H "+%s"`
expiredSub=`date -u -v -1d "+%s"`
expired=`date -u -v -365d "+%s"`

echo  'uid: '$1 'email: '$2
if [[ -e $3 ]]
then
	echo "Encrypting tickets with key file: $3"
fi

printf "\n\n\n"

echo "Expired trial ticket for mac:"
$path/create_ticket.sh $deviceId $expiredSub 1 1 1 1 $1 $2 $3

printf "\n\n"

echo "Active (expires in 1 day) trial ticket for mac:"
$path/create_ticket.sh $deviceId $activeOneDayLeft 1 1 1 1 $1 $2 $3

printf "\n\n"

echo "Active trial ticket for mac:"
$path/create_ticket.sh $deviceId $active 1 1 1 1 $1 $2 $3

printf "\n\n"

echo "Expired Version individual basic ticket for mac:"
$path/create_ticket.sh $deviceId $expired 2 1 1 1 $1 $2 $3

printf "\n\n"

echo "Expired Subscription individual basic ticket for mac:"
$path/create_ticket.sh $deviceId $expiredSub 2 1 1 1 $1 $2 $3

printf "\n\n"

echo "Active (sub expires in less than 10 days) individual basic ticket for mac:"
$path/create_ticket.sh $deviceId $activeAlmostExpired 2 1 1 1 $1 $2 $3

printf "\n\n"

echo "Active (sub expires in less than 1 days) individual basic ticket for mac:"
$path/create_ticket.sh $deviceId $activeLessThanOneDayLeft 2 1 1 1 $1 $2 $3

printf "\n\n"

echo "Active individual basic ticket for mac:"
$path/create_ticket.sh $deviceId $active 2 1 1 1 $1 $2 $3

printf "\n"
