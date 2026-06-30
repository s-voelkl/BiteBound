package com.example.bitebound

import com.example.bitebound.data.Credentials
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

// the connect button is only enabled when the credentials are "complete"
class CredentialsTest {

    // host/user/topics are pre-filled, so out of the box only the password is missing
    @Test
    fun defaultsAreNotCompleteUntilPasswordIsSet() {
        assertFalse(Credentials().isComplete)
        assertTrue(Credentials(password = "secret").isComplete)
    }

    @Test
    fun blankHostMakesItIncomplete() {
        assertFalse(Credentials(host = "", password = "secret").isComplete)
    }

    @Test
    fun blankPlayerNameMakesItIncomplete() {
        assertFalse(Credentials(playerName = "", password = "secret").isComplete)
    }

    // port has to be a real port number
    @Test
    fun portHasToBeInRange() {
        assertFalse(Credentials(port = 0, password = "secret").isComplete)
        assertFalse(Credentials(port = 70000, password = "secret").isComplete)
        assertTrue(Credentials(port = 1, password = "secret").isComplete)
        assertTrue(Credentials(port = 65535, password = "secret").isComplete)
    }
}
