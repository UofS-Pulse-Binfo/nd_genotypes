<?php
namespace Tests\api;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use  Tests\DatabaseSeeders\GeneticMarkerSeeder;

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
   * Tests nd_genotypes_get_marker().
   *
   * @group markers
   */
  public function testGetMarker() {

    // There is configuration that allows the relationship connecting
    // markers/variants to be either variant => marker or marker => variant.
    // We need to test both configurations.
    foreach(['subject', 'object'] as $variant_position) {

      // Now create a fake term to remove assumptions.
      $term = factory('chado.cvterm')->create();
      $rel_type = $term->cvterm_id;
      variable_set('nd_genotypes_rel_type_id', $rel_type);
      variable_set('nd_genotypes_rel_position', $variant_position);

      // Create a marker/variant dataset.
      $seeder = new GeneticMarkerSeeder();
      $data = $seeder->up();

      // Retrieve the marker.
      $marker = nd_genotypes_get_marker($data['variant']['variant_id']);

      $this->assertNotFalse($marker, "Unable to return the marker.");

      // There should only be a single marker...
      $this->assertIsObject($marker, "There should only be a single marker.");
      $this->assertEquals($data['marker']['marker_id'], $marker->feature_id,
        "We retrieved something but it was not the marker we expected.");

      // Now add a second marker.
      $new_marker = $seeder->addMarker();
      $markers = nd_genotypes_get_marker($data['variant']['variant_id'], FALSE, FALSE);

      $this->assertNotFalse($markers, "Unable to return the marker set.");

      // There should now be two markers...
      $this->assertIsArray($markers, "There should be two markers.");
      $this->assertArrayContainsObjectAttributeValue(
        $markers, 'feature_id', $new_marker['marker_id'],
        "We retrieved a set but it did not contain the marker we expected.");

      // Now we should be able to ask for only one even though there are two.
      // This is the default so do not pass additional params.
      $marker = nd_genotypes_get_marker($data['variant']['variant_id']);
      $this->assertNotFalse($marker, "Unable to return the marker.");
      $this->assertIsObject($marker, "There should only be a single marker.");

      // We can also ask for only the IDs so lets test that.
      // Retrieve full marker set (2 markers).
      $markers = nd_genotypes_get_marker($data['variant']['variant_id'], TRUE, FALSE);
      $this->assertIsArray($markers, "There should be two markers.");
      $this->assertEquals(
        [$data['marker']['marker_id'], $new_marker['marker_id']],
        $markers,
        "The marker array does not contain the IDs we expect."
      );

      // Retrieve only a single marker ID.
      $marker_id = nd_genotypes_get_marker($data['variant']['variant_id'], TRUE);
      $this->assertIsNumeric($marker_id, "We did not recieve a valid marker ID.");
    }
  }

  /**
   * Tests nd_genotypes_get_type_id().
   *
   * @group markers
   */
  public function testGetTypeId() {
    $this->markTestIncomplete('This test has not been implemented yet.');
  }

  /**
   * Tests nd_genotypes_get_marker_type().
   *
   * @group markers
   */
  public function testGetMarkerType() {
    $this->markTestIncomplete('This test has not been implemented yet.');
  }

  /**
   * Tests nd_genotypes_get_variant().
   *
   * @group markers
   */
  public function testGetVariant() {
    $this->markTestIncomplete('This test has not been implemented yet.');
  }

  /**
   * Tests nd_genotypes_get_variants_for_sequence().
   *
   * @group markers
   */
  public function testGetVariantsForSequence() {
    $this->markTestIncomplete('This test has not been implemented yet.');
  }

  private function assertArrayContainsObjectAttributeValue($theArray, $attribute, $value) {
    foreach($theArray as $arrayItem) {
        if($arrayItem->$attribute == $value) {
            return TRUE;
        }
    }
    return FALSE;
  }
}
