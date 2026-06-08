project('peak-tracker', 'cpp',
        version: '0.1',
        default_options: ['cpp_std=c++23'])

sdbusplus_dep = dependency('sdbusplus')
boost_dep = dependency('boost')

executable('mfg-peak-tracker',
           'main.cpp',
           dependencies: [sdbusplus_dep, boost_dep],
           install: true)