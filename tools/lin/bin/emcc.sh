#!/bin/bash

if [[ -z "${EMSCRIPTEN_HOME}" ]]; then
	echo "EMSCRIPTEN_HOME is not defined" > /dev/stderr
	exit 1
fi

YELLOW='1;33'
RED='1;31'

BIN="${EMSCRIPTEN_HOME}/emcc"
ARGS="$@"

test "$verbose" != 0 && echo "${BIN} ${ARGS}"

${BIN} ${ARGS} 2>&1 >/dev/null | GREP_COLOR="${YELLOW}" grep -E -i --color 'warning|$'

exit ${PIPESTATUS[0]}
