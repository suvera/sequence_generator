#include <cstdlib>
#include <iostream>
#include <string.h>

using namespace std;

// main
int main(const int argc, char *argv[])
{
    if (argc < 3) {
        cout << "Please provide path and ini configuration file and PID file\n";
        exit(1);
    }
    
    const char *begin = "#!/bin/bash\n"
        "# \n"
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
    ;
    
    const char *end = "\n\n"
        "DAEMON=\"$SEQUENCERD\"\n"
        "if [ \"$CONFIG_INI\" != \"\" ]\n"
        "then\n"
        "DAEMON=\"$DAEMON -c $CONFIG_INI\"\n"
        "fi\n\n"
        "\n"
        "# sanity check\n"
        "\n"
        "if [ ! -x \"$SEQUENCERD\" ] ; then\n"
        "    echo \"No $SEQUENCERD package installed\"\n"
        "    exit 0\n"
        "fi\n"
        "\n"
        "\n"
        "# Start the service\n"
        "start() {\n"
        "    echo \"Starting $NAME server: \"\n"
        "    PID=\n"
        "    STARTED=0\n"
        "    if [ -e $PIDFILE ]; then\n"
        "        PID=$(cat $PIDFILE)\n"
        "    fi\n\n"
        "    if [ \"$PID\" != \"\" ]; then\n"
        "        ps -p $PID\n"
        "        if [ $? != 1 ]; then \n"
        "            STARTED=1\n"
        "        fi\n"
        "    fi\n\n"
        "    if [ \"$STARTED\" == \"0\" ] \n"
        "    then\n"
        "        $DAEMON -D\n"
        "        if [ \"$?\" != \"0\" ] \n"
        "        then\n"
        "            echo \"Not Started.\"\n"
        "        else\n"
        "        echo \"Successfully Started!\"\n"
        "        fi\n"
        "    else\n"
        "        echo \"Already Running!.\"\n"
        "    fi\n\n"
        "    echo\n"
        "}\n"
        "\n"
        "# Restart the service\n"
        "stop() {\n"
        "    echo \"Stopping $NAME server: \"\n"
        "    PID=\n"
        "    STARTED=0\n"
        "    if [ -e $PIDFILE ]; then\n"
        "        echo \"PID file exist!\"\n"
        "        PID=$(cat $PIDFILE)\n"
        "        echo \"PID: $PID\"\n"
        "    fi\n\n"
        "    if [ \"$PID\" != \"\" ]; then\n"
        "        ps -p $PID\n"
        "        if [ \"$?\" != \"1\" ]; then \n"
        "            STARTED=1\n"
        "        fi\n"
        "    fi\n\n"
        "    if [ \"$STARTED\" == \"0\" ] \n"
        "    then\n"
        "        echo \"not running!\"\n"
        "    else\n"
        "        kill $PID\n"
        "        echo \"Stopped!\"\n"
        "        rm -f $PIDFILE\n"
        "    fi\n\n"
        "    echo\n"
        "}\n"
        "\n"
        "# Check status of the service\n"
        "status() {\n"
        "    ps -p `cat $PIDFILE`\n"
        "    if [ \"$?\" == \"1\" ] \n"
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
        "        echo \"Usage: $0 {start|stop|restart|reload|status}\"\n"
        "        exit 1\n"
        "esac\n"
        "exit 0\n"
    ;
    
    string s = begin;
    s.append("SEQUENCERD=");
    s.append(argv[1]);
    s.append("\n");

    s.append("CONFIG_INI=");
    s.append(argv[2]);
    s.append("\n");

    s.append("PIDFILE=");
    s.append(argv[3]);
    s.append("\n");

    s.append(end);
    
    cout << s;
    
    return 0;
}