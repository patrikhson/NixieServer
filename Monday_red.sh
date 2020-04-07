#!/bin/sh -xv
#./CLITool clock <./input &
while true; do
  hour=`date +"%H"`
  min=`date +"%M"`

    if [ $min -eq 0 ]; then
      weekday=`date "+%a"`
      case $weekday in
        Mon)
	  echo leds 20 0 0 >./input
	  ;;
        Tue)
	  echo leds 14 6 0 >./input
	  ;;
        Wed)
	  echo leds 0 20 0 >./input
	  ;;
        Thu)
	  echo leds 0 10 10 >./input
	  ;;
        Fri)
	  echo leds 0 0 20 >./input
	  ;;
        Sat)
	  echo leds 5 0 18 >./input
	  ;;
        Sun)
	  echo leds 10 0 10 >./input
	  ;;
      esac
      sleep 1
    fi
# Don't display anything between 2am and 6am
  if [ $hour -ge 2 -a $hour -lt 6 ]; then
    echo "      " >./input
    sleep 30
    continue
  elif [ $hour -ne 0 -o $min -gt 0 ]; then
    x=`curl http://192.168.0.47/cgi-bin/gettemp.pl?sensor=out 2>/dev/null`
    out=`dc -e "$x 1 / p"`
    x=`curl http://192.168.0.47/cgi-bin/gettemp.pl?sensor=in 2>/dev/null`
    in=`dc -e "$x 1 / p"`
    if [ $in -eq 0 -o $out -eq 0 ]; then
      sleep 10
      continue
    fi

    echo "dots 0 0" >./input
    sleep 1
    if [ $out -ge 100 ]; then
      echo "${in} ${out}" >./input
    else
      echo "${in}  ${out}" >./input
    fi
    sleep 5
  fi

# echo "dots 1 1" >./input
  sleep 1
  echo "clock" >./input
  sleep 10
done
