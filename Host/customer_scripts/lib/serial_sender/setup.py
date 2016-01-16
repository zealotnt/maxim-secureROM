#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# src/setup.py
#
# ----------------------------------------------------------------------------
# Copyright © 2009, Maxim Integrated Products
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Maxim Integrated Products nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY MAXIM INTEGRATED PRODUCTS ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL MAXIM INTEGRATED PRODUCTS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------
#
# Created on: Dec 16, 2009
# Author: Grégory Romé <gregory.rome@maxim-ic.com>
#
# ---- Subversion keywords (need to set the keyword property)
# $Rev:: 482           $:  Revision of last commit
# $Author:: gregory.ro#$:  Author of last commit
# $Date:: 2009-12-18 0#$:  Date of last commit
#
""" Win32 binary generator

:Author: Grégory Romé - gregory.rome@maxim-ic.com
:Organization: Maxim Integrated Products
:Copyright: Copyright © 2009, Maxim Integrated Products
:License: BSD License - http://www.opensource.org/licenses/bsd-license.php
"""

from distutils.core import setup
import sys
import serial_sender

if sys.platform == 'win32':
    import py2exe
    # If run without args, build executables, in quiet mode.
    if len(sys.argv) == 1:
        sys.argv.append("py2exe")
        sys.argv.append("-q")
    setup(name=serial_sender.PROG,
          version=serial_sender.VERSION,
          description=serial_sender.PROG,
          author=serial_sender.AUTHOR,
          url='http://www.maxim-ic.com',
          download_url='http://www.maxim-ic.com',
          console=['serial_sender.py'],
          options={
                    "py2exe":{
                            "unbuffered": True,
                            "optimize": 2,
                            "bundle_files": 1,
							#"includes": ["System","pdb"]
                    }
          },
		  

    )
else:
    print 'No setup for Linux platform'
