
Find a variant in a trait-implicated region
=============================================

**Research Question:**

Through other analysis you have a region of the genome which likely contributes to a specific phenotype for your trait of interest. Now you would like to find a causative or at least correlated sequence variant. For this purpose you know at least two germplasm with differing phenotypes which you have genotypic data available for.

Fictional Example:
 - Trait: ``FAIRness``
 - Region of interest: ``non:150-300``
 - Germplasm with FAIRness: ``placeat libero cupiditate et``
 - Germplasm without FAIRness: ``omnis fuga molestiae et``

**Data:**

This example uses simulated data for the fictional species Tripalus databasica. You can generate similar using the Tripal Test Suite as described here: :ref:`demo-instructions`. You can also use your own data by importing it into your Tripal site using the `genotype loader <https://github.com/UofS-Pulse-Binfo/genotypes_loader>`_.

Step #1: Find genotypic data for your reference germplasm
-----------------------------------------------------------

 - Go to ``[yourtripalsite]/chado/genotype/[Genus]`` (e.g. ``http://localhost/tripal-DEV/chado/genotype/Tripalus``) to access the genotype matrix tool for the genus of the germplasm you are interested in.
 - Enter the name of each germplasm you are interested in by typing it in the textfield labelled germplasm. Then check the correct species is selected to the right of the textbox. To add more then one germplasm click the green + button.
 - Each time you click the green + button or search, the genotypic data for the listed germplasm will be shown.

.. image:: gmatrix-poly-region.1.png

Step #2: Restrict the Sequence Variants to polymorphic between your germplasm
------------------------------------------------------------------------------

 - Underneath germplasm, there is a filter to restrict to polymorphic variants. This filter compares two germplasm and only shows variants with different genotypic calls.
 - For our example, we would select ``placeat libero cupiditate et`` in the first drop down and ``omnis fuga molestiae et`` in the second drop-down to see only sequence variants with differing genotypes (i.e polymorphic variants) between these two germplasm.
 - Click Search to see the results.

.. image:: gmatrix-poly-region.2.png

Step #3: Restrict to you trait-implicated Region of the Genome.
-----------------------------------------------------------------

- The second section of the filter criteria available for the genotype matrix allows you to enter the region of the genome you are interested in. Once you click search, the genotype matrix will only show sequence variants found in this region.
- In our example, the region of interest is non:150-300. To enter this we place ``non`` for the ``Sequence Name``, ``150`` for the start position and ``300`` for the end position.

.. image:: gmatrix-poly-region.3.png

Step 4: (Optionally): Restrict to specific variants.
------------------------------------------------------

- Say further analysis shows that particular sequence variants in that region are more likely to contribute to your phenotype of interest.
- You can enter the specific variant names by expanding the ``Additional Filter criteria`` section then clicking Search.

.. image:: gmatrix-poly-region.4.png
