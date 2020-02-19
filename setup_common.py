import sys

__doc__ = '''
This package contains implementation of Linux watchdog device API for Python
(see Documentation/watchdog/watchdog-api.txt in Linux kernel source tree).

Included extension module provides wrappers for ioctl commands as well as simple
write access to the device.
'''.strip()

if sys.platform != 'linux2':
    sys.stderr.write(('Currently this module can only be used on Linux.'
                     ' Sorry.\n'))
    sys.exit(1)

classifiers = '''
Development Status :: 3 - Alpha
Intended Audience :: Developers
Intended Audience :: System Administrators
License :: OSI Approved :: Python Software Foundation License
Operating System :: POSIX :: Linux
Programming Language :: Python
Programming Language :: C
Topic :: System :: Hardware
Topic :: System :: Networking :: Monitoring :: Hardware Watchdog
Topic :: System :: Operating System Kernels :: Linux
'''.strip().split('\n')

watchdogdev = Extension('watchdogdev', sources=['src/watchdogdev.c'])

setup(name='watchdogdev',
      version='0.11',
      author='Timur Izhbulatov',
      author_email='timochka@gmail.com',
      description='Implementation of Linux watchdog device API',
      long_description=__doc__,
      license='http://opensource.org/licenses/PythonSoftFoundation.php',
      url='http://timka.org/watchdogdev/',
      platforms=['linux2'],
      ext_modules=[watchdogdev])
