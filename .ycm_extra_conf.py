# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import imp
import os
import shlex
import sys
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO

old_bytecode = sys.dont_write_bytecode
sys.dont_write_bytecode = True

path = os.path.join(os.path.dirname(__file__), 'real-mach')

if not os.path.exists(path):
    path = os.path.join(os.path.dirname(__file__), 'config.status')
    config = imp.load_module('_buildconfig', open(path), path, ('', 'r', imp.PY_SOURCE))
    path = os.path.join(config.topsrcdir, 'real-mach')
mach_module = imp.load_module('_mach', open(path), path, ('', 'r', imp.PY_SOURCE))

sys.dont_write_bytecode = old_bytecode

def FlagsForFile(filename):
    mach = mach_module.get_mach()
    out = StringIO()

    # Mach calls sys.stdout.fileno(), so we need to fake it when capturing it.
    # Returning an invalid file descriptor does the trick.
    out.fileno = lambda: -1
    out.encoding = None
    mach.run(['compileflags', filename], stdout=out, stderr=out)

    flag_list = shlex.split(out.getvalue())

    return {
        'flags': final_flags,
        'do_cache': True
    }
