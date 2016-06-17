<?php

// Print out header row, if option was selected.
if ($options['header']) {
  print implode($options['separator'], $header) . "\r\n";
}
