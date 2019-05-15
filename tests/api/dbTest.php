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
   * Tests nd_genotypes_get_postgresql_version() 
   * and nd_genotypes_recheck_postgresql_version().
   */
  public function testGetPsqlVersion() {

    // Retrieve the version.
    // Ensure we are not simply testing a cached version.
    nd_genotypes_recheck_postgresql_version(); 
    $result = nd_genotypes_get_postgresql_version();

    // Check that a version was returned.
    $this->assertNotEmpty($result, "Unable to detect PostgreSQL version.");
    $this->assertIsFloat($result, "Returned PosgreSQL version was not a float.");

    // Check that it is the correct version.
    $full_version = db_query('SELECT version()')->fetchField();
    $this->assertContains( (string)$result, $full_version,
      "The reported version is not in the original version string from PosgreSQL.");

    // Check that rechecking the version does not change it.
    $first = $result;
    nd_genotypes_recheck_postgresql_version();
    $second = nd_genotypes_get_postgresql_version();
    $this->assertEquals($first, $second, 
      "Re-checking the postgreSQL version should not change the result.");
  }
}
