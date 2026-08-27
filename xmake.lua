add_rules('mode.debug', 'mode.release')

package('xdgcpp')
  set_urls('https://github.com/Grumbel/xdgcpp.git')
  add_deps('cmake')

  on_install(
    function (package)
      import('package.tools.cmake').install(package)
    end
  )
package_end()

add_requires('xdgcpp')
add_requires('toml++', { system = false })

set_allowedplats('linux')
set_languages('c++26')

target('clamshell')
  set_kind('binary')
  add_files('src/**.cc')
  add_packages(
    'xdgcpp',
    'toml++'
  )
  if is_mode("debug") then
    add_defines("DEBUG")
  elseif is_mode("release") then
    add_defines("RELEASE")
  end
  add_defines('TOML_HEADER_ONLY=1')
