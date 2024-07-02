#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
GREEN='\033[0;32m'
NC='\033[0m' # No Color

cd "$DIR" || {
  echo "Redirection failed!"
  exit 1
}

echo -e "${GREEN}Building Azure dataset processing tool...${NC}"
./gradlew clean shadowJar
echo -e "${GREEN}Building Azure dataset processing tool...done${NC}"

# $JAVA_HOME/bin/native-image -jar build/libs/azure-dataset-1.0-SNAPSHOT.jar dataset-processor
