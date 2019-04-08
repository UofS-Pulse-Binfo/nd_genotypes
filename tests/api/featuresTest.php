<?php
namespace Tests\api;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

/**
 * Tests the following API functions:
 *   - nd_genotypes_get_marker()
 *   - nd_genotypes_get_type_id()
 *   - nd_genotypes_get_marker_type()
 *   - nd_genotypes_get_variant()
 *   - nd_genotypes_get_variants_for_sequence()
 */
class featuresTest extends TripalTestCase {
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
