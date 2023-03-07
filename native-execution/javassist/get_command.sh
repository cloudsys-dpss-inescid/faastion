PACKAGE=$1
JAVA_AGENT=$2
TOOL=$3
ENTRYPOINT=$4

if [ ! -v PACKAGE ] || [ ! -v JAVA_AGENT ] || [ ! -v TOOL ] || [ ! -v ENTRYPOINT ]; then
  echo "Invalid arguments"
  echo "./get_command.sh <package-name> <path-to-java-agent> <tool-name> <path-to-entrypoint>"
fi

# Get all directories with a *.class file in them and replace '/' with '.'
packages=$(jar -tf ${PACKAGE} | grep -oE ".*\/[^/]+\.class" | sed 's#/[^/]*$##; s#/#.#g' | sort -u)

# String of packages separated by commas
printf -v output '%s,' $packages
output=${output%,}

echo "java -cp ${PACKAGE} -javaagent:${JAVA_AGENT}=${TOOL}:${output}:output ${ENTRYPOINT}"
