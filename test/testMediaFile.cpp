/**
 * @file testMediaFile.cpp
 * @brief Unit Tests for MediaFile class using Google Test framework.
 * @details Covers constructors, getters, setters, validity check, and operators.
 * @author Architecture Team
 */

#include <gtest/gtest.h>
#include "../src/model/MediaFile.h"

using namespace Model;

/**
 * @brief Test Case: Default Constructor
 * Scenario: Verify default constructor creates an invalid MediaFile.
 */
TEST(MediaFileTest, DefaultConstructor) {
    MediaFile file;

    EXPECT_EQ(file.getFilename(), "");
    EXPECT_EQ(file.getPath(), "");
    EXPECT_EQ(file.getDuration(), 0u);
    EXPECT_EQ(file.getArtist(), "");
    EXPECT_EQ(file.getAlbum(), "");
    EXPECT_FALSE(file.isValid());
}

/**
 * @brief Test Case: Parameterized Constructor (Minimal)
 * Scenario: Create MediaFile with only filename and path.
 */
TEST(MediaFileTest, ParameterizedConstructorMinimal) {
    MediaFile file("song.mp3", "/home/user/music/song.mp3");

    EXPECT_EQ(file.getFilename(), "song.mp3");
    EXPECT_EQ(file.getPath(), "/home/user/music/song.mp3");
    EXPECT_EQ(file.getDuration(), 0u);  // Default
    EXPECT_EQ(file.getArtist(), "");     // Default
    EXPECT_EQ(file.getAlbum(), "");      // Default
    EXPECT_TRUE(file.isValid());
}

/**
 * @brief Test Case: Parameterized Constructor (Full)
 * Scenario: Create MediaFile with all parameters.
 */
TEST(MediaFileTest, ParameterizedConstructorFull) {
    MediaFile file("bohemian.mp3", "/music/bohemian.mp3", 355, "Queen", "A Night at the Opera");

    EXPECT_EQ(file.getFilename(), "bohemian.mp3");
    EXPECT_EQ(file.getPath(), "/music/bohemian.mp3");
    EXPECT_EQ(file.getDuration(), 355u);
    EXPECT_EQ(file.getArtist(), "Queen");
    EXPECT_EQ(file.getAlbum(), "A Night at the Opera");
    EXPECT_TRUE(file.isValid());
}

/**
 * @brief Test Case: Setters
 * Scenario: Verify all setters work correctly.
 */
TEST(MediaFileTest, Setters) {
    MediaFile file;

    file.setFilename("test.mp3");
    file.setPath("/path/to/test.mp3");
    file.setDuration(180);
    file.setArtist("Artist Name");
    file.setAlbum("Album Name");

    EXPECT_EQ(file.getFilename(), "test.mp3");
    EXPECT_EQ(file.getPath(), "/path/to/test.mp3");
    EXPECT_EQ(file.getDuration(), 180u);
    EXPECT_EQ(file.getArtist(), "Artist Name");
    EXPECT_EQ(file.getAlbum(), "Album Name");
    EXPECT_TRUE(file.isValid());
}

/**
 * @brief Test Case: isValid() - Valid File
 * Scenario: File with both filename and path is valid.
 */
TEST(MediaFileTest, IsValidTrue) {
    MediaFile file("a.mp3", "/a.mp3");
    EXPECT_TRUE(file.isValid());
}

/**
 * @brief Test Case: isValid() - Empty Filename
 * Scenario: File without filename is invalid.
 */
TEST(MediaFileTest, IsValidEmptyFilename) {
    MediaFile file;
    file.setPath("/some/path.mp3");
    EXPECT_FALSE(file.isValid());
}

/**
 * @brief Test Case: isValid() - Empty Path
 * Scenario: File without path is invalid.
 */
TEST(MediaFileTest, IsValidEmptyPath) {
    MediaFile file;
    file.setFilename("song.mp3");
    EXPECT_FALSE(file.isValid());
}

/**
 * @brief Test Case: Equality Operator
 * Scenario: Two files with same path are equal.
 */
TEST(MediaFileTest, EqualityOperator) {
    MediaFile file1("song.mp3", "/music/song.mp3", 200, "Artist1", "Album1");
    MediaFile file2("different.mp3", "/music/song.mp3", 300, "Artist2", "Album2");

    // Same path = equal (even if other fields differ)
    EXPECT_TRUE(file1 == file2);
    EXPECT_FALSE(file1 != file2);
}

/**
 * @brief Test Case: Inequality Operator
 * Scenario: Two files with different paths are not equal.
 */
TEST(MediaFileTest, InequalityOperator) {
    MediaFile file1("song.mp3", "/music/song1.mp3");
    MediaFile file2("song.mp3", "/music/song2.mp3");

    EXPECT_FALSE(file1 == file2);
    EXPECT_TRUE(file1 != file2);
}

/**
 * @brief Test Case: Copy Constructor
 * Scenario: Copy a MediaFile and verify independence.
 */
TEST(MediaFileTest, CopyConstructor) {
    MediaFile original("test.mp3", "/test.mp3", 100, "Artist", "Album");
    MediaFile copy = original;

    EXPECT_EQ(copy.getFilename(), original.getFilename());
    EXPECT_EQ(copy.getPath(), original.getPath());
    EXPECT_EQ(copy.getDuration(), original.getDuration());
    EXPECT_EQ(copy.getArtist(), original.getArtist());
    EXPECT_EQ(copy.getAlbum(), original.getAlbum());

    // Modify copy, original should be unchanged
    copy.setFilename("modified.mp3");
    EXPECT_NE(copy.getFilename(), original.getFilename());
}

/**
 * @brief Test Case: Copy Assignment
 * Scenario: Assign one MediaFile to another.
 */
TEST(MediaFileTest, CopyAssignment) {
    MediaFile original("test.mp3", "/test.mp3", 100);
    MediaFile assigned;

    assigned = original;

    EXPECT_EQ(assigned.getFilename(), original.getFilename());
    EXPECT_EQ(assigned.getPath(), original.getPath());
}

/**
 * @brief Test Case: Move Constructor
 * Scenario: Move a MediaFile.
 */
TEST(MediaFileTest, MoveConstructor) {
    MediaFile original("test.mp3", "/test.mp3", 100);
    MediaFile moved = std::move(original);

    EXPECT_EQ(moved.getFilename(), "test.mp3");
    EXPECT_EQ(moved.getPath(), "/test.mp3");
    // original is in valid but unspecified state after move
}

/**
 * @brief Test Case: Special Characters in Strings
 * Scenario: Handle special characters in filename, path, artist, album.
 */
TEST(MediaFileTest, SpecialCharacters) {
    MediaFile file(
        "bài hát 日本語.mp3",
        "/home/用户/音乐/bài hát 日本語.mp3",
        240,
        "Nghệ sĩ 🎵",
        "Album ♫"
    );

    EXPECT_EQ(file.getFilename(), "bài hát 日本語.mp3");
    EXPECT_EQ(file.getArtist(), "Nghệ sĩ 🎵");
    EXPECT_TRUE(file.isValid());
}

/**
 * @brief Test Case: Long Strings
 * Scenario: Handle very long filenames and paths.
 */
TEST(MediaFileTest, LongStrings) {
    std::string longName(1000, 'x');
    std::string longPath = "/" + longName + ".mp3";

    MediaFile file(longName + ".mp3", longPath);

    EXPECT_EQ(file.getFilename().length(), 1004);  // 1000 + ".mp3"
    EXPECT_TRUE(file.isValid());
}
