#!/usr/bin/env python

from distutils.core import setup

import fscc


setup(name='fscc',
      version=fscc.__version__,
      description='Commtech FSCC Python API',
      author='William Fagan',
      author_email='willf@commtech-fastcom.com',
      url='http://code.google.com/p/pyfscc/',
      provides=["pyfscc (%s)" % fscc.__version__],
      packages=['fscc'],
     )
