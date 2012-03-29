#!/usr/bin/env python

from distutils.core import setup

import fscc_cli


setup(name='fscc_cli',
      version=fscc_cli.__version__,
      description='Commtech FSCC CLI',
      author='William Fagan',
      author_email='willf@commtech-fastcom.com',
      url='http://code.google.com/p/fscc-cli/',
      requires=["pyfscc (>1.0.2)", "argparse"],
      scripts=['fscc_cli/fscc-cli', 'fscc_cli/fscc-set-clock'],
     )
