<?php
namespace Tests\api;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

/**
 * Tests the following API functions:
 *   - tripal_germplasm_has_nd_genotypes()
 *   - nd_genotypes_has_genotypes()
 *   - nd_genotypes_markup_sequence_with_variants()
 */
class genotypesTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Basic test example.
   * Tests must begin with the word "test".
   * See https://phpunit.readthedocs.io/en/latest/ for more information.
   */
  public function testBasicExample() {
    $this->assertTrue(true);
  }
}
