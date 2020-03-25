<?php
namespace Tests\api;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use  Tests\DatabaseSeeders\GeneticMarkerSeeder;

/**
 * Tests the following API functions:
 *   - tripal_germplasm_has_nd_genotypes()
 *   - nd_genotypes_has_genotypes()
 *   - nd_genotypes_markup_sequence_with_variants()
 *   - nd_genotypes_get_variants_for_sequence()
 */
class genotypesTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Tests tripal_germplasm_has_nd_genotypes().
   *
   * @group genotypes
   */
  public function testGermplasmHasGenotypes() {

    // Check when the mview tables do not exist to make sure it doesn't break.
    $has_genotypes = tripal_germplasm_has_nd_genotypes(
      $germplasm_id, uniqid());
    $this->assertFalse($has_genotypes,
      "There should not even be tables for the mviews yet.");

    // Create a marker/variant dataset.
    $seeder = new GeneticMarkerSeeder();
    $data = $seeder->up();
    $partition = strtolower($data['organism']['genus']);
    $this->createTables($partition);
    $mview = $seeder->syncMviews($partition);

    // Check if there are genotypes for a non-existent
    // germplasm for our organism.
    $germplasm_id = 1;
    $has_genotypes = tripal_germplasm_has_nd_genotypes(
      $germplasm_id, $partition);
    $this->assertFalse($has_genotypes,
      "This germplasm doesn't exist in our organism so it shouldn't have genotypes.");

    // Now add genotypes to our marker and check again.
    $genotype_data = $seeder->addGenotypes(1);
    $mview = $seeder->syncMviews($partition);
    $germplasm_id = $genotype_data[0]['sample']->stock_id;
    $has_genotypes = tripal_germplasm_has_nd_genotypes(
      $germplasm_id, $partition);
    $this->assertTrue($has_genotypes,
      "This germplasm was just created with genotypes so it does have some.");
  }

  /**
   * Tests nd_genotypes_has_genotypes().
   *
   * @group genotypes
   */
  public function testFeatureHasGenotypes() {

    // Check when the mview tables do not exist to make sure it doesn't break.
    $has_genotypes = nd_genotypes_has_genotypes(1, uniqid());
    $this->assertFalse($has_genotypes,
      "There should not even be tables for the mviews yet.");

    // Create a marker/variant dataset.
    $seeder = new GeneticMarkerSeeder();
    $data = $seeder->up();
    $partition = strtolower($data['organism']['genus']);
    $this->createTables($partition);
    $new_marker = $seeder->addMarker();
    $partition = strtolower($data['organism']['genus']);
    $mview = $seeder->syncMviews($partition);

    // Check if there are genotypes for a non-existent
    // marker for our organism.
    $has_genotypes = nd_genotypes_has_genotypes(1, $partition);
    $this->assertFalse($has_genotypes,
      "This marker doesn't exist in our organism so it shouldn't have genotypes.");

    // Now add genotypes to our marker and check again.
    $genotype_data = $seeder->addGenotypes(1);
    $mview = $seeder->syncMviews($partition);
    $marker_id = $genotype_data[0]['marker']->feature_id;
    $has_genotypes = nd_genotypes_has_genotypes(
      $marker_id, $partition);
    $this->assertTrue($has_genotypes,
      "This marker was just created with genotypes so it does have some.");

  }

  /**
   * Tests nd_genotypes_get_variants_for_sequence().
   *
   * @group genotypes
   */
  public function testGetVariantsForSequence() {
    $start = 100;
    $end = 100000;

    // Always call it first to make sure it doesn't break when
    // the tables don't yet exist.
    $variants = nd_genotypes_get_variants_for_sequence('fred', $start, $end, uniqid());

    // Create a marker/variant dataset.
    $seeder = new GeneticMarkerSeeder();
    $data = $seeder->up();
    $partition = strtolower($data['organism']['genus']);
    $this->createTables($partition);
    $genotype_data = $seeder->addGenotypes(1);
    $partition = strtolower($data['organism']['genus']);
    $mview = $seeder->syncMviews($partition);
    $srcfeature_name = $data['chromosome']['record']->name;

    // Then check if we can pull the single variant back out.
    $variants = nd_genotypes_get_variants_for_sequence($srcfeature_name, $start, $end, $partition);

    $this->assertNotFalse($variants, "Check that the function succeeded.");
    $this->assertCount(1, $variants, "There should only be a single variant.");
    $this->assertArrayContainsObjectAttributeValue(
      $variants, 'variant_id', $mview['variant']['variant_id'],
      "We had something returned but it was not the variant we expected.");

    // Now create a second variant with the same organism.
    $seeder2 = new GeneticMarkerSeeder();
    $data2 = $seeder2->up(
      $data['organism']['organism_id'],
      $data['chromosome']['chromosome_id']
    );
    $genotype_data2 = $seeder2->addGenotypes(1);
    $mview2 = $seeder2->syncMviews($partition);

    // Then check if we can pull out both variants.
    $variants = nd_genotypes_get_variants_for_sequence($srcfeature_name, $start, $end, $partition);

    $this->assertNotFalse($variants, "Check that the function succeeded.");
    $this->assertCount(2, $variants, "There should be two variants.");
    $this->assertArrayContainsObjectAttributeValue(
      $variants, 'variant_id', $mview['variant']['variant_id'],
      "The two results didn't contain the first variant we created.");
    $this->assertArrayContainsObjectAttributeValue(
      $variants, 'variant_id', $mview2['variant']['variant_id'],
      "The two results didn't contain the second variant we created.");

  }

  /**
   * Tests nd_genotypes_markup_sequence_with_variants().
   *
   * @group genotypes
   */
  public function testMarkupSequence() {

    // Always call it first to make sure it doesn't break when
    // the tables don't yet exist.
    putenv("TRIPAL_SUPPRESS_ERRORS=TRUE");
    $sequence = nd_genotypes_get_variants_for_sequence([], [], uniqid());
    putenv("TRIPAL_SUPPRESS_ERRORS");
    $this->assertFalse($sequence, "We shouldn't be able to pull up a sequence without any parameters.");

    // Create a marker/variant dataset.
    $seeder = new GeneticMarkerSeeder();
    $data = $seeder->up();
    $partition = strtolower($data['organism']['genus']);
    $this->createTables($partition);
    $genotype_data = $seeder->addGenotypes(1);
    $partition = strtolower($data['organism']['genus']);
    $mview = $seeder->syncMviews($partition);
    $srcfeature_name = $data['chromosome']['record']->name;

    // Now create a second variant with the same organism.
    $seeder2 = new GeneticMarkerSeeder();
    $data2 = $seeder2->up(
      $data['organism']['organism_id'],
      $data['chromosome']['chromosome_id']
    );
    $genotype_data2 = $seeder2->addGenotypes(1);
    $mview2 = $seeder2->syncMviews($partition);

    // Check when we supply all the parameters in the ideal case...
    $values = [
      'srcfeature_id' => $data['chromosome']['chromosome_id'],
      'srcfeature_name' => $srcfeature_name,
      'variant_id' => $data['variant']['variant_id'],
      'sequence' => str_repeat('N', 700),
      'sequence_start' => 50,
      'sequence_end' => 650,
      'sequence_strand' => -1,
    ];
    $sequence = nd_genotypes_markup_sequence_with_variants(
      $values, [], $partition);

    $allele = $genotype_data[0]['allele'];
    if ($allele[0] == $allele[1]) {
      $expanded = "/\[\w\/N\]/";
    }
    else {
      $expanded = "/\[\w\/\w\/N\]/";
    }
    $this->assertRegExp($expanded, $sequence,
      "The allele should be marked up in the sequence.");

    $complemented = nd_genotypes_complement_calls($mview2['calls'][0]['allele_call']);
    $code = nd_genotypes_get_IUPAC_code($complemented);
    $this->assertStringContainsString($code, $sequence,
      "The IUPAC code of the second variant should be present.");

    // Finally check we recieve an error if we do not provide sequence coords.
    unset($values['sequence_start'], $values['sequence_end']);
    putenv("TRIPAL_SUPPRESS_ERRORS=TRUE");
    $sequence = nd_genotypes_markup_sequence_with_variants(
      $values, [], $partition);
    putenv("TRIPAL_SUPPRESS_ERRORS");
    $this->assertEquals(str_repeat('N', 700), $sequence,
      "The sequence should not contain any variants.");

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

  /**
   * HELPER: Creates tables necessary for sync'ing germplasm.
   */
  private function createTables($partition) {

    $calls_table = 'mview_ndg_'.$partition.'_calls';
    nd_genotypes_create_mview_ndg_calls($calls_table);
    $variant_table = 'mview_ndg_'.$partition.'_variants';
    nd_genotypes_create_mview_ndg_variants($variant_table);
    $germplasm_table = 'mview_ndg_germplasm_genotyped';
    nd_genotypes_create_mview_ndg_germplasm_genotyped($germplasm_table);
  }
}
