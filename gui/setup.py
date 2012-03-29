#!/usr/bin/env python

from distutils.core import setup

import fscc_gui


setup(name='fscc_gui',
      version=fscc_gui.__version__,
      description='Commtech FSCC GUI',
      author='William Fagan',
      author_email='willf@commtech-fastcom.com',
      url='http://code.google.com/p/fscc-gui/',
      requires=["pyfscc (>1.0.1)"],
      scripts=['fscc_gui/fscc-gui'],
      packages=['fscc_gui'],
      package_dir={'fscc_gui': 'fscc_gui'},
      package_data={'fscc_gui': ['layouts/*.glade']},
      data_files=[('/usr/share/applications', ['fscc_gui/fscc-gui.desktop']),
                  ('/usr/share/icons/hicolor/48x48/apps/', ['fscc_gui/fscc-gui.png']),
                 ]
     )
