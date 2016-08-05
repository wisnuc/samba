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

    # pdbedit -e smbpasswd:/backup/samba/sambaUsers.bak
    # pdbedit -e tdbsam:/backup/samba/sambaUsers.bak
    # Save this user
    echo $name $passwd >> /srv/samba/backup/sambaUsers.bak
}

### import: import a configuration file
# Arguments:
#   file - file to import
# Return: user(s) added to container
import()
{
    local name
    local passwd
    local file="${1}"
    while read name passwd; do
        useradd "$name" -M
        (echo $passwd ; echo $passwd ) | smbpasswd -s -a $name
    done < <(cut -d: -f1,2 --output-delimiter=' ' $file)
    # pdbedit -i smbpasswd:$file
}

mkdir -p /srv/samba/backup/
echo "" > /srv/samba/backup/sambaUsers.bak

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
    ionice -c 3 nmbd -D
    exec ionice -c 3 smbd -FS </dev/null
else
    exit 0
fi
