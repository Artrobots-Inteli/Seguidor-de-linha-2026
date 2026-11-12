#!/usr/bin/env bash
exec "$(dirname "${BASH_SOURCE[0]}")/dev.sh" build "$@"
