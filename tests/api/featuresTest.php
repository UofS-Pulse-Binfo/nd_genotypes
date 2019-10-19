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

    // Existing type to use for testing purposes.
    $type_id = chado_query(
      'SELECT cvterm_id FROM chado.cvterm limit 1')->fetchField();

    // First delete the variables and check we get a default.
    variable_del('nd_genotypes_markerprop_type_id');
    variable_del('nd_genotypes_variantprop_type_id');
    variable_del('nd_genotypes_rel_type_id');
    variable_del('nd_genotypes_rel_position');
    variable_del('nd_genotypes_subproject_rel_type_id');
    variable_del('nd_genotypes_subproject_rel_position');
    variable_del('nd_genotypes_stock_rel_type_id');
    variable_del('nd_genotypes_stock_rel_position');

    // For each category.
    $type_categories = [
      'Marker Type',
      'Variant Type',
      'Feature Relationship',
      'Project Relationship',
      'Stock Relationship'
    ];
    foreach ($type_categories as $category) {

      if (strpos($category, "Type") !== FALSE) {
        $type_id = nd_genotypes_get_type_id($category);
        $this->assertIsNumeric($type_id, "Unable to retrieve the $category.");
      }
      elseif ($category == 'Project Relationship') {
        $type = nd_genotypes_get_type_id($category);
        $this->assertNull($type['type_id'], 'There should not be a default project relationship.');
      }
      else {
        $type = nd_genotypes_get_type_id($category);
        $this->assertIsNumeric($type['type_id'],
          "Unable to retrieve the $category type_id.");
      }
    }

    // Now test if we pass in an incorrect category we get false.
    $type = nd_genotypes_get_type_id(uniqid());
    $this->assertFalse($type, 'We should get false if we try an unsupported category.');
  }

  /**
   * Tests nd_genotypes_get_marker_type().
   *
   * @group markers
   */
  public function testGetMarkerType() {

    // Create a marker/variant dataset.
    $seeder = new GeneticMarkerSeeder();
    $data = $seeder->up();
    //print_r($data);

    $type = nd_genotypes_get_marker_type($data['marker']['marker_id']);
    $this->assertNotFalse($type, 'We were unable to retrieve the marker type.');
    $this->assertEquals($data['marker_type']['type'], $type,
      "The marker type did not match what we expected.");

  }

  /**
   * Tests nd_genotypes_get_variant().
   *
   * @group markers
   */
  public function testGetVariant() {
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

      // Retrieve the variant.
      $variant = nd_genotypes_get_variant($data['marker']['marker_id']);

      $this->assertNotFalse($variant, "Unable to return the variant.");

      // There should only be a single variant...
      $this->assertIsObject($variant, "There variant should be an object.");
      $this->assertEquals($data['variant']['variant_id'], $variant->feature_id,
        "We retrieved something but it was not the variant we expected.");

      // We can also ask for only the IDs so lets test that.
      $variant_id = nd_genotypes_get_variant($data['marker']['marker_id'], TRUE);
      $this->assertIsNumeric($variant_id, "We did not recieve a valid variant ID.");
    }
  }

  /**
   * A helper assert to check if an array of objects has a specific value.
   */
  private function assertArrayContainsObjectAttributeValue($theArray, $attribute, $value) {
    foreach($theArray as $arrayItem) {
        if($arrayItem->$attribute == $value) {
            return TRUE;
        }
    }
    return FALSE;
  }
}
