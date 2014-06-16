<?php

/**
 * @file
 * Programmers interface to ND Genotypes
 */

/**
 * Determine whether a given stock has genotypes in the nd_genotype_experiment table.
 *
 * @param $stock_id
 *   The unique ID of the stock you want to check
 *
 * @return
 *   TRUE if there are genotypes available and FALSE otherwise
 */
function tripal_germplasm_has_nd_genotypes($stock_id) {
  $query = "SELECT count(*) as nd_genotypes FROM chado.nd_genotype_experiment WHERE stock_id=:stock_id";
  $result = chado_query($query, array(':stock_id' => $stock_id))->fetchField();

  if ($result > 0) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}
