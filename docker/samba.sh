#!/bin/bash

set -e                              # should exit if any statement returns a non-true return value
set -o nounset                      # Treat unset variables as an error

### adduser: add a user
# Arguments:
#   name - for user
#   password - for user
# Return: user added to container
adduser()
{
    local name="${1}"
    local passwd="${2}"
    useradd "$name" -M

    (echo $passwd ; echo $passwd ) | smbpasswd -s -a $name
    smbpasswd -e $name

    # Save this user
    pdbedit -Lw -u "$name" >> /srv/samba/backup/sambaUsers.bak
}

### import: import a configuration file
# just like:
# aaaaa:1000:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:32ED87BDB5FDC5E9CBA88547376818D4:[U          ]:LCT-57A84611:
# bbbbb:1001:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:4057B60B514C5402DDE3D29A1845C366:[U          ]:LCT-57A84611:

# Arguments:
#   file - file to import
# Return: user(s) added to container
import()
{
    local name
    local id
    local file="${1}"
    while read name id; do
        useradd "$name" -M -u "$id"
    done < <(cut -d: -f1,2 --output-delimiter=' ' $file)
    pdbedit -i smbpasswd:$file
}

mkdir -p /srv/samba/backup/

while getopts ":i:nu:" opt; do
    case "$opt" in
        i) import "$OPTARG" ;;
        n) NMBD="true" ;;
        u) eval adduser $(sed 's|:| |g' <<< $OPTARG) ;;
        "?") echo "Unknown option: -$OPTARG"; usage 1 ;;
        ":") echo "No argument value for option: -$OPTARG"; usage 2 ;;
    esac
done
shift $(( OPTIND - 1 ))

if [[ $# -ge 1 && -x $(which $1 2>&-) ]]; then
    exec "$@"
elif [[ $# -ge 1 ]]; then
    echo "ERROR: command not found: $1"
    exit 13
elif ps -ef | egrep -v grep | grep -q smbd; then
    echo "Service is already running."
elif [[ ${NMBD:-""} ]]; then
    ionice -c 3 nmbd -D && ionice -c 3 smbd -D
    while true; do sleep 1000; done
else
    exit 0
fi
