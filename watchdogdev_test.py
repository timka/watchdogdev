from __future__ import print_function
from watchdogdev import *


def test():
    wd = watchdog('/dev/watchdog')
    print('fileno:', wd.fileno())
    wd.close()

    wd = watchdog('/dev/watchdog')

    print('options:', wd.options)
    assert(wd.options == None)
    print('firmware_version:', wd.firmware_version)
    assert(wd.firmware_version == None)
    print('identity:', wd.identity)
    assert(wd.identity == None)
    print()

    wd.close()

    wd = watchdog('/dev/watchdog')

    wd.get_support()

    print('options:', wd.options)
    for name in list(globals()):
        if not name.startswith('WDIOF_'):
            continue
        value = globals()[name]
        print('%s:' % name, wd.options & value == value)

    print()
    print('firmware_version:', wd.firmware_version)
    print('identity:', wd.identity)
    print()

    wd.close()

    wd = watchdog('/dev/watchdog')

    print('keep_alive:', end=' ')
    try:
        print(wd.keep_alive())
    except IOError as e:
        print(e)

    print('get_status:', end=' ')
    try:
        print(wd.get_status())
    except IOError as e:
        print(e)

    print('get_boot_status:', end=' ')
    try:
        print(wd.get_boot_status())
    except IOError as e:
        print(e)

    print('get_temp:', end=' ')
    try:
        print(wd.get_temp())
    except IOError as e:
        print(e)

    timeout = 60
    print('get_timeout:', end=' ')
    try:
        timeout = wd.get_timeout()
        print(timeout)
    except IOError as e:
        print(e)

    print('get_pretimeout:', end=' ')
    try:
        print(wd.get_pretimeout())
    except IOError as e:
        print(e)

    print('get_time_left:', end=' ')
    try:
        print(wd.get_time_left())
    except IOError as e:
        print(e)

    print('set_options:', end=' ')
    try:
        for i in range(0, 65536):
            wd.set_options(i)
            assert(wd.get_options == i)
        print('OK')
    except IOError as e:
        print(e)

    print('set_timeout:', end=' ')
    try:
        for i in range(1, 65536):
            wd.set_timeout(i)
            assert(wd.get_timeout() == i)
        wd.set_timeout(timeout)
        print('OK')
    except IOError as e:
        print(e)

    print('set_pretimeout:', end=' ')
    try:
        for i in range(1, 65536):
            wd.set_pretimeout(i)
            assert(wd.get_pretimeout == i)
        wd.set_pretimeout(0)
        print('OK')
    except IOError as e:
        print(e)

    print('magic_close:', end=' ')
    try:
        wd.magic_close()
        print('OK')
    except IOError as e:
        print(e)


if __name__ == '__main__':
    test()
