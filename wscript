#!/usr/bin/env python

import subprocess
import time
import sys

top = '.'
out = 'build'

APPNAME = 'jogma'
VERSION = '0.0.0'

def separator():
    # just for output prettifying
    # print() (as a function) doesn't work in python <2.7
    sys.stdout.write('\n')

#
# dep checking functions
#

def pkg_check(ctx, pkg, what='--cflags --libs'):
    ctx.check_cfg(
        package=pkg, args=what, uselib_store=pkg.upper())

def check_jack(ctx):
    pkg_check(ctx, 'jack')

def check_flac(ctx):
    pkg_check(ctx, 'flac', '--libs')

#
# waf stuff
#

def options(opt):
    opt.load('compiler_c')

def configure(ctx):
    separator()

    ctx.load('compiler_c')
    ctx.load('gnu_dirs')

    separator()
    check_jack(ctx)
    check_flac(ctx)
    separator()

    ctx.env.append_unique('CFLAGS', [
        '-std=gnu99', '-Wall', '-Werror', '-Wextra', '-Wcast-align',
        '-Wno-missing-field-initializers', '-Wno-unused-parameter',
        '-ffunction-sections', '-fdata-sections', '-ggdb'])

    [ctx.define('_JOGMA_VERSION_' + v[0], v[1], quote=False)
            for v in zip(('MAJOR', 'MINOR', 'PATCH_LEVEL'), VERSION.split('.'))]

    ctx.define('_JOGMA_GIT_REVISION',
            subprocess.check_output(['git', 'rev-parse', '--verify', '--short', 'HEAD'])
            .decode().strip())

    ctx.write_config_header('jogma_build_config.h')
    ctx.define('_GNU_SOURCE', 1)

def build(ctx):
    ctx.recurse('third-party')
    ctx.recurse('src')
