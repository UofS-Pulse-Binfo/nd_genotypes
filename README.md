# Natural Diversity Genotypes
This module provides a specialized interface to genotypic information stored in Chado including adding summaries of this information to feature pages and providing a powerful marker by germplasm matrix.

The 3.x branch of this module switches from using Drupal Views and moves towards a much more efficient framework. This process was initiated to ensure this modue can handle large datasets (tested with 5 billion genotype calls: 5 million variants by 1000 germplasm). 

Furthermore, the new branch of this module stores it's data in a custom gneotype_call gathering table rather than the Chado natural diversity module. This change was also undertaken in the interests of efficiency and is documented in the (wiki)[https://github.com/UofS-Pulse-Binfo/nd_genotypes/wiki/How-to-Store-your-Data]. It is currently a fully PostgreSQL solution, although it is being developed to allow use of other data backends in the future as needed.

## Functionality
1. Configuration allowing admin to specify controlled vocabulary terms used for marker and variant types, as well as, relationship types.
2. Genotype matrix: A table-based search allowing users to retrieve a specific variant by germplasm slice of genotype data
3. A genotype summary per marker and variant shown as it's own pane on marker and variant feature pages.
4. A variant marked-up sequence per marker and variant providing the user with 500bp flanking sequence with flanking variants highlighted via linked IUPAC codes.
5. Custom form element types providing an interface for users to select multiple germplasm and choose a pairwise combination of germplasm.
6. Sync functionality allowing admin to cache genotype calls and speed up the afore mentioned functionality.
7. Countless efficiency improvements including optimized queries and well-choosen indexes.

## Dependencies
1. Tripal 3.x
2. Tripal Download API

## Installation
1. Install the dependencies listed above.
2. Install this module as you would any other Drupal module.

## Usage
1. Configure this module by going to Administration » Tripal » Extensions » Natural Diversity Genotypes. Specifically, you need to set the various controlled vocabulary terms that this module will expect.
2. Load data using the [genotype loader](https://github.com/UofS-Pulse-Binfo/genotypes_loader). Since the Genotype loader is not yet released, we highly suggest test loading each dataset on a development site.
3. You can access the genotype matrix at [your drupal site]/chado/genotypes/[genus].
4. You should see a "Genotypes" and updated "Sequences" pane on Marker and Variant pages.

### Genotype Matrix
This module provides genotype search functionality that allows users to select which germplasm and variants they are interested in and be shown a colour-coded variant by germplasm table which can be further filtered by marker/variant type and to only show polymorphic variants (pairwise comparison choosen by the user). After filtering to their desired dataset, the user can download the table as a tab-delimited file.

The following screenshot shows the form the user is presented with for choosing filter criteria:
![genotypematrix_form](https://cloud.githubusercontent.com/assets/1566301/19090330/1d656d6c-8a3b-11e6-8776-6f3c1e10e18b.png)

The following screenshot shows the table of genotype calls presented to the user based on their filter criteria:
![genotypematrix_table](https://cloud.githubusercontent.com/assets/1566301/19090346/2e83e0b0-8a3b-11e6-9ad3-9574aa88b7e5.png)

### Marker/Variant Pages
This module adds two panes to marker and variant pages:

1. A "Genotypes" pane which summarizes the calls recorded for a given marker/variant. Specifically, a pie chart depicts the ratio of calls recorded.
2. A "Sequences" pane which shows 500bp flanking sequence for the current marker/variant with the alleles for the current marker highlighted as [A/B] and all other variants located in the flanking sequence indicated using their IUPAC codes. This display provides an easy means for researchers to design new markers if the old technology is no longer desirable.
