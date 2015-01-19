#!/bin/bash

#The OTX Server run script

echo "The OTX Server run script"
VERSION="1.00a"

case "$1" in
  --help|-h)
      echo >&2 "Usage $0 <options>"
      echo >&2 " Where options are:"
      echo >&2 " -h|--help|start|stop|restart|kill|--version";
      if [ -f otxserver ];
      then
        ./otxserver --help
      fi
      ;;
   start|s)
     if [ -f otxserver ];
     then
        echo >&2 "Running The OTX Server v$VERSION"
        ./otxserver
        echo >&2 "Exited with code $?"
      else
         echo >&2 "Could not start The OTX Server"
      fi
     ;;
   restart)
     echo "Restarting..."
     killall -9 otxserver
     if [ -f otxserver ];
     then
        ./otxserver
		echo >&2 "Exited with code $?"
     else
        echo >&2 "Failed"
     fi
     ;;
   stop|kill)
     killall -9 otxserver
     ;;
   --version)
     echo "v$VERSION"
     exit
     ;;
   *)
      echo >&2 "Usage $0 <options>"
      echo >&2 " Where options are:"
      echo >&2 " -h|--help|start|stop|restart|kill";
      ;;
esac
