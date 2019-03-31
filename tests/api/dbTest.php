<?php
namespace Tests\api;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

/**
 * Tests the following API functions:
 *   - ndg_mview_query()
 *   - ndg_mview_tables_exist()
 *   - ndg_remote_chado_copy()
 *   - nd_genotypes_recheck_postgresql_version()
 *   - nd_genotypes_get_postgresql_version()
 *   - nd_genotypes_drop_indexes()
 */
class dbTest extends TripalTestCase {
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
