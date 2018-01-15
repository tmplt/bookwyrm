#
# Collection of cmake utility functions
#

# message_colored : Outputs a colorized message {{{

function(message_colored message_level text color)
  string(ASCII 27 esc)
  message(${message_level} "${esc}[${color}m${text}${esc}[0m")
endfunction()

# }}}
