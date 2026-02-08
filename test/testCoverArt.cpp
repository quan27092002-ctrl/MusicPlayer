/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/testCoverArt.cpp
 * DESCRIPTION: Unit tests for CoverArt class.
 */

#include <gtest/gtest.h>
#include "model/mediafile/CoverArt.h"
#include <vector>
#include <cstdint>

class CoverArtTest : public ::testing::Test {
protected:
    Model::CoverArt coverArt;
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(CoverArtTest, DefaultConstruction) {
    Model::CoverArt ca;
    EXPECT_FALSE(ca.hasCoverArt());
    EXPECT_EQ(ca.size(), 0u);
}

TEST_F(CoverArtTest, ConstructionWithData) {
    std::vector<uint8_t> data = {0x89, 0x50, 0x4E, 0x47}; // PNG header
    Model::CoverArt ca(data);
    
    EXPECT_TRUE(ca.hasCoverArt());
    EXPECT_EQ(ca.size(), 4u);
}

TEST_F(CoverArtTest, ConstructionWithEmptyData) {
    std::vector<uint8_t> data;
    Model::CoverArt ca(data);
    
    EXPECT_FALSE(ca.hasCoverArt());
    EXPECT_EQ(ca.size(), 0u);
}

// ============================================================================
// SetCoverArt Tests
// ============================================================================

TEST_F(CoverArtTest, SetCoverArt) {
    std::vector<uint8_t> data = {0xFF, 0xD8, 0xFF, 0xE0}; // JPEG header
    coverArt.setCoverArt(data);
    
    EXPECT_TRUE(coverArt.hasCoverArt());
    EXPECT_EQ(coverArt.size(), 4u);
}

TEST_F(CoverArtTest, SetCoverArtEmptyData) {
    std::vector<uint8_t> data = {0x01, 0x02};
    coverArt.setCoverArt(data);
    EXPECT_TRUE(coverArt.hasCoverArt());
    
    // Set empty data
    std::vector<uint8_t> empty;
    coverArt.setCoverArt(empty);
    EXPECT_FALSE(coverArt.hasCoverArt());
}

TEST_F(CoverArtTest, SetCoverArtOverwrite) {
    std::vector<uint8_t> data1 = {0x01, 0x02};
    std::vector<uint8_t> data2 = {0x03, 0x04, 0x05, 0x06};
    
    coverArt.setCoverArt(data1);
    EXPECT_EQ(coverArt.size(), 2u);
    
    coverArt.setCoverArt(data2);
    EXPECT_EQ(coverArt.size(), 4u);
}

// ============================================================================
// GetCoverArt Tests
// ============================================================================

TEST_F(CoverArtTest, GetCoverArtReturnsCorrectData) {
    std::vector<uint8_t> data = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A};
    coverArt.setCoverArt(data);
    
    const auto& retrieved = coverArt.getCoverArt();
    EXPECT_EQ(retrieved.size(), data.size());
    EXPECT_EQ(retrieved[0], 0x89);
    EXPECT_EQ(retrieved[1], 0x50);
}

TEST_F(CoverArtTest, GetCoverArtWhenEmpty) {
    const auto& retrieved = coverArt.getCoverArt();
    EXPECT_TRUE(retrieved.empty());
}

// ============================================================================
// HasCoverArt Tests
// ============================================================================

TEST_F(CoverArtTest, HasCoverArtInitiallyFalse) {
    EXPECT_FALSE(coverArt.hasCoverArt());
}

TEST_F(CoverArtTest, HasCoverArtAfterSet) {
    std::vector<uint8_t> data = {0x01};
    coverArt.setCoverArt(data);
    EXPECT_TRUE(coverArt.hasCoverArt());
}

// ============================================================================
// Clear Tests
// ============================================================================

TEST_F(CoverArtTest, Clear) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    coverArt.setCoverArt(data);
    EXPECT_TRUE(coverArt.hasCoverArt());
    
    coverArt.clear();
    EXPECT_FALSE(coverArt.hasCoverArt());
    EXPECT_EQ(coverArt.size(), 0u);
}

TEST_F(CoverArtTest, ClearAlreadyEmpty) {
    coverArt.clear();
    EXPECT_FALSE(coverArt.hasCoverArt());
    EXPECT_EQ(coverArt.size(), 0u);
}

// ============================================================================
// Size Tests
// ============================================================================

TEST_F(CoverArtTest, SizeZeroWhenEmpty) {
    EXPECT_EQ(coverArt.size(), 0u);
}

TEST_F(CoverArtTest, SizeMatchesData) {
    std::vector<uint8_t> data(1024, 0xFF);
    coverArt.setCoverArt(data);
    EXPECT_EQ(coverArt.size(), 1024u);
}

TEST_F(CoverArtTest, SizeLargeData) {
    std::vector<uint8_t> data(100000, 0x00);
    coverArt.setCoverArt(data);
    EXPECT_EQ(coverArt.size(), 100000u);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(CoverArtTest, SingleByteData) {
    std::vector<uint8_t> data = {0xFF};
    coverArt.setCoverArt(data);
    
    EXPECT_TRUE(coverArt.hasCoverArt());
    EXPECT_EQ(coverArt.size(), 1u);
    EXPECT_EQ(coverArt.getCoverArt()[0], 0xFF);
}

TEST_F(CoverArtTest, SetClearSetPattern) {
    std::vector<uint8_t> data1 = {0x01, 0x02};
    std::vector<uint8_t> data2 = {0x03, 0x04, 0x05};
    
    coverArt.setCoverArt(data1);
    EXPECT_EQ(coverArt.size(), 2u);
    
    coverArt.clear();
    EXPECT_EQ(coverArt.size(), 0u);
    
    coverArt.setCoverArt(data2);
    EXPECT_EQ(coverArt.size(), 3u);
}
