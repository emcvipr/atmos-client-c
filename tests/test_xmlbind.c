/*
 * test_xmlbind.c
 *
 *  Created on: Dec 7, 2012
 *      Author: cwikj
 */

#include "seatest.h"
#include "test.h"
#include "atmos_token_policy.h"

#define TEST_XML_INPUT \
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"\
    "<policy>\n" \
    "    <expiration>2012-12-01T12:00:00.000Z</expiration>\n"\
    "    <max-uploads>1</max-uploads>\n"\
    "    <source>\n"\
    "        <allow>127.0.0.0/24</allow>\n"\
    "    </source>\n"\
    "    <content-length-range from=\"10\" to=\"11000\"/>\n"\
    "    <form-field name=\"x-emc-redirect-url\"/>\n"\
    "    <form-field name=\"x-emc-meta\" optional=\"true\">\n"\
    "        <matches>^(\\w+=\\w+)|((\\w+=\\w+),(\\w+, \\w+))$</matches>\n"\
    "    </form-field>\n"\
    "</policy>\n"

// A little different than the original.  Libxml2 has different indents and
// we don't do expiration down to the ms.
#define TEST_XML_OUTPUT \
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"\
    "<policy>\n"\
    "  <expiration>2012-12-01T12:00:00Z</expiration>\n"\
    "  <max-uploads>1</max-uploads>\n"\
    "  <source>\n"\
    "    <allow>127.0.0.0/24</allow>\n"\
    "  </source>\n"\
    "  <content-length-range from=\"10\" to=\"11000\"/>\n"\
    "  <form-field name=\"x-emc-redirect-url\"/>\n"\
    "  <form-field name=\"x-emc-meta\" optional=\"true\">\n"\
    "    <matches>^(\\w+=\\w+)|((\\w+=\\w+),(\\w+, \\w+))$</matches>\n"\
    "  </form-field>\n"\
    "</policy>\n"

// Check XML->struct (unmarshal)
void test_read_xml() {
    const char *xml = TEST_XML_INPUT;
    Atmos_policyType *p;

    p = Atmos_policyType_unmarshal(xml);

    assert_true(p != NULL);
    if(!p) {
        return;
    }
    assert_int_equal(1, p->expiration_set);
    assert_int64t_equal(1354363200, p->expiration);
    assert_int_equal(1, p->max_uploads_set);
    assert_int64t_equal(1, p->max_uploads);
    assert_int_equal(0, p->max_downloads_set);
    assert_true(p->source != NULL);
    if(p->source == NULL) return;
    assert_int_equal(1, p->source->allow_count);
    assert_int_equal(0, p->source->disallow_count);
    assert_string_equal("127.0.0.0/24", p->source->allow[0]);
    assert_int64t_equal(10, p->content_length_range->from);
    assert_int64t_equal(11000, p->content_length_range->to);
    assert_int_equal(2, p->form_field_count);
    assert_string_equal("x-emc-redirect-url", p->form_field[0].name);
    assert_int_equal(0, p->form_field[0].optional_set);
    assert_int_equal(0, p->form_field[0].contains_count);
    assert_int_equal(0, p->form_field[0].matches_count);
    assert_int_equal(0, p->form_field[0].ends_with_count);
    assert_int_equal(0, p->form_field[0].eq_count);
    assert_string_equal("x-emc-meta", p->form_field[1].name);
    assert_int_equal(1, p->form_field[1].optional_set);
    assert_int_equal(1, p->form_field[1].optional);
    assert_int_equal(0, p->form_field[1].contains_count);
    assert_int_equal(1, p->form_field[1].matches_count);
    assert_int_equal(0, p->form_field[1].ends_with_count);
    assert_int_equal(0, p->form_field[1].eq_count);
    assert_string_equal("^(\\w+=\\w+)|((\\w+=\\w+),(\\w+, \\w+))$", p->form_field[1].matches[0]);

    Atmos_policyType_destroy(p);
    free(p);
}

// Check struct->XML (marshal)
void test_write_xml() {
    Atmos_policyType p;
    xmlChar *xml;

    Atmos_policyType_init(&p);

    p.expiration = 1354363200;
    p.expiration_set = 1;
    p.max_uploads = 1;
    p.max_uploads_set = 1;
    p.source = Atmos_sourceType_init(malloc(sizeof(Atmos_sourceType)));
    p.source->allow_count = 1;
    p.source->allow = malloc(sizeof(char*));
    p.source->allow[0] = strdup("127.0.0.0/24");
    p.content_length_range = Atmos_contentLengthRangeType_init(malloc(sizeof(Atmos_contentLengthRangeType)));
    p.content_length_range->from = 10;
    p.content_length_range->to = 11000;
    p.form_field_count = 2;
    p.form_field = malloc(2 * sizeof(Atmos_formFieldType));
    Atmos_formFieldType_init(&p.form_field[0]);
    Atmos_formFieldType_init(&p.form_field[1]);
    p.form_field[0].name = strdup("x-emc-redirect-url");
    p.form_field[1].name = strdup("x-emc-meta");
    p.form_field[1].optional_set = 1;
    p.form_field[1].optional = 1;
    p.form_field[1].matches_count = 1;
    p.form_field[1].matches = malloc(sizeof(char*));
    p.form_field[1].matches[0] = strdup("^(\\w+=\\w+)|((\\w+=\\w+),(\\w+, \\w+))$");

    xml = Atmos_policyType_marshal(&p);

    assert_string_equal(TEST_XML_OUTPUT, (char*)xml);

    Atmos_policyType_destroy(&p);
    xmlFree(xml);
}

// Round-trip check.
// Build a struct, serialize it to XML and back and check it.
void test_readwrite_xml() {
    Atmos_policyType p;
    xmlChar *xml;
    Atmos_policyType *pp;

    Atmos_policyType_init(&p);

    p.expiration = 1354363200;
    p.expiration_set = 1;
    p.max_uploads = 1;
    p.max_uploads_set = 1;
    p.source = Atmos_sourceType_init(malloc(sizeof(Atmos_sourceType)));
    p.source->allow_count = 1;
    p.source->allow = malloc(sizeof(char*));
    p.source->allow[0] = strdup("127.0.0.0/24");
//    p.content_length_range.from = 10;
//    p.content_length_range.to = 11000;
    p.form_field_count = 2;
    p.form_field = malloc(2 * sizeof(Atmos_formFieldType));
    Atmos_formFieldType_init(&p.form_field[0]);
    Atmos_formFieldType_init(&p.form_field[1]);
    p.form_field[0].name = strdup("x-emc-redirect-url");
    p.form_field[1].name = strdup("x-emc-meta");
    p.form_field[1].optional_set = 1;
    p.form_field[1].optional = 1;
    p.form_field[1].matches_count = 1;
    p.form_field[1].matches = malloc(sizeof(char*));
    p.form_field[1].matches[0] = strdup("^(\\w+=\\w+)|((\\w+=\\w+),(\\w+, \\w+))$");

    xml = Atmos_policyType_marshal(&p);
    pp = Atmos_policyType_unmarshal((char*)xml);

    assert_true(pp != NULL);
    if(!pp) {
        return;
    }
    assert_int_equal(1, pp->expiration_set);
    assert_int64t_equal(1354363200, pp->expiration);
    assert_int_equal(1, pp->max_uploads_set);
    assert_int64t_equal(1, pp->max_uploads);
    assert_int_equal(0, pp->max_downloads_set);
    assert_true(pp->source != NULL);
    if(pp->source == NULL) return;
    assert_int_equal(1, pp->source->allow_count);
    assert_int_equal(0, pp->source->disallow_count);
    assert_string_equal("127.0.0.0/24", pp->source->allow[0]);
//    assert_int64t_equal(10, pp->content_length_range.from);
//    assert_int64t_equal(11000, pp->content_length_range.to);
    assert_int_equal(2, pp->form_field_count);
    assert_string_equal("x-emc-redirect-url", pp->form_field[0].name);
    assert_int_equal(0, pp->form_field[0].optional_set);
    assert_int_equal(0, pp->form_field[0].contains_count);
    assert_int_equal(0, pp->form_field[0].matches_count);
    assert_int_equal(0, pp->form_field[0].ends_with_count);
    assert_int_equal(0, pp->form_field[0].eq_count);
    assert_string_equal("x-emc-meta", pp->form_field[1].name);
    assert_int_equal(1, pp->form_field[1].optional_set);
    assert_int_equal(1, pp->form_field[1].optional);
    assert_int_equal(0, pp->form_field[1].contains_count);
    assert_int_equal(1, pp->form_field[1].matches_count);
    assert_int_equal(0, pp->form_field[1].ends_with_count);
    assert_int_equal(0, pp->form_field[1].eq_count);
    assert_string_equal("^(\\w+=\\w+)|((\\w+=\\w+),(\\w+, \\w+))$", pp->form_field[1].matches[0]);

    Atmos_policyType_destroy(&p);
    Atmos_policyType_destroy(pp);
    free(pp);

    xmlFree(xml);
}

void test_xmlbind_suite() {
    test_fixture_start();

    start_test_msg("test_read_xml");
    run_test(test_read_xml);
    start_test_msg("test_write_xml");
    run_test(test_write_xml);
    start_test_msg("test_readwrite_xml");
    run_test(test_readwrite_xml);

    test_fixture_end();
}
