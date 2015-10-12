package com.ubirch

import java.io.File

import org.specs2.mutable.Specification
import spray.testkit.Specs2RouteTest
import spray.http._
import StatusCodes._

class HalloServiceSpec extends Specification with Specs2RouteTest with HalloService {
  def actorRefFactory = system
  
  "HalloService" should {

    "return a greeting for GET requests to the root path" in {
      Get() ~> routes ~> check {
        responseAs[String] must contain("HALLO")
      }
    }

    "stores a file for upload" in {
      Post("/101", HttpEntity(HttpData(new File("src/test/resources/example.ogg")))) ~> routes ~> check {
        responseAs[String] must contain("101")
      }
    }

    "serves a file for download" in {
      Get("/101") ~> routes ~> check {
        responseAs[Array[Byte]].length mustEqual 35778
      }
    }

    "leave GET requests to other paths unhandled" in {
      Get("/kermit") ~> routes ~> check {
        handled must beFalse
      }
    }

    "return a MethodNotAllowed error for PUT requests to the root path" in {
      Put() ~> sealRoute(routes) ~> check {
        status === MethodNotAllowed
        responseAs[String] === "HTTP method not allowed, supported methods: GET"
      }
    }
  }
}
