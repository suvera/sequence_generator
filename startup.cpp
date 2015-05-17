#include <cstdlib>
#include <iostream>
#include <string.h>

using namespace std;

// main
int main(const int argc, char *argv[])
{
    if (argc == 1) {
        cout << "Please provide path\n";
        exit(1);
    }
    
    const char *begin = "#!/bin/sh\n"
        "\n"
        "# chkconfig: 35 90 12\n"
        "# Description: Start the sequencer server\n"
        "# \n"
        "# Get function from functions library\n"
        "if [ -e /etc/init.d/functions ] ; then\n"
        ". /etc/init.d/functions\n"
        "fi\n"
        "\n"
        "DESC=\"sequencer server\"\n"
        "NAME=sequencer\n"
        "DAEMON="
    ;
    
    const char *end = "\n\n"
        "PIDFILE=$($DAEMON -I)\n"
        "\n"
        "# sanity check\n"
        "[ -x $DAEMON ] || exit 0\n"
        "\n"
        "if [ ! -x $DAEMON ] ; then\n"
        "    echo \"No $DAEMON package installed\"\n"
        "    exit 0\n"
        "fi\n"
        "\n"
        "if [ -z \"$PIDFILE\" ] ; then\n"
        "    echo \"ERROR: Could not find $NAME $PID_FILE\"\n"
        "    exit 2\n"
        "fi\n"
        "\n"
        "\n"
        "# Start the service\n"
        "start() {\n"
        "    initlog -c \"echo -n Starting $NAME server: \"\n"
        "    ps -p `cat $PIDFILE`\n"
        "    if [ $? = 1 ] \n"
        "    then\n"
        "        $DAEMON -D\n"
        "        success $\"$NAME server startup\"\n"
        "    else\n"
        "        echo \"Already Running.\"\n"
        "    fi\n"
        "    echo\n"
        "}\n"
        "\n"
        "# Restart the service\n"
        "stop() {\n"
        "    initlog -c \"echo -n Stopping $NAME server: \"\n"
        "    killproc -p $PIDFILE $DAEMON\n"
        "    echo\n"
        "}\n"
        "\n"
        "# Check status of the service\n"
        "status() {\n"
        "    ps -p `cat $PIDFILE`\n"
        "    if [ $? = 1 ] \n"
        "    then\n"
        "        echo \"Not Running.\"\n"
        "    else\n"
        "        echo \"Running.\"\n"
        "    fi\n"
        "}\n"
        "\n"
        "\n"
        "### main logic ###\n"
        "case \"$1\" in\n"
        "  start)\n"
        "        start\n"
        "        ;;\n"
        "  stop)\n"
        "        stop\n"
        "        ;;\n"
        "  status)\n"
        "        status\n"
        "        ;;\n"
        "  restart|reload|condrestart)\n"
        "        stop\n"
        "        start\n"
        "        ;;\n"
        "  *)\n"
        "        echo $\"Usage: $0 {start|stop|restart|reload|status}\"\n"
        "        exit 1\n"
        "esac\n"
        "exit 0\n"
    ;
    
    string s = begin;
    s.append(argv[1]);
    s.append(end);
    
    cout << s;
    
    return 0;
}