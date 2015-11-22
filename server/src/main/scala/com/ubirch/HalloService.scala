package com.ubirch

import java.io.{File, FileOutputStream}
import java.nio.file.{Paths, Path, StandardCopyOption, Files}
import java.text.SimpleDateFormat
import java.util.{Date, Calendar, TimeZone}

import akka.actor.{Actor, ActorLogging}
import akka.util.Timeout
import spray.can.Http
import spray.http._
import spray.httpx.SprayJsonSupport

import scala.concurrent.Future
import scala.concurrent.duration._

case class DeleteFile(file: File)

class HalloServiceActor extends Actor with ActorLogging {
  implicit val timeout: Timeout = 1.second // for the actor 'asks'

  final val dataDir = new File("data")
  var status: Map[String, Date] = Map()

  // 866224021727127 - Stephan (grosse Puppe)
  // 866224021723183 - Stephan (kleine Puppe)
  // 860719020980821 - Berlin (Hallo#0-1)
  // 866224021716898 - Stefan
  var connections: Map[String, Seq[String]] = Map(
    "866224021723183" -> Seq("866224021727127"),
    "866224021727127" -> Seq("866224021723183"),
    "860719020980821" -> Seq("866224021716898"),
    "866224021716898" -> Seq("860719020980821")
  )

  def receive = {
    case _: Http.Connected => sender ! Http.Register(self)

    case HttpRequest(HttpMethods.GET, Uri.Path("/"), _, _, _) =>
      log.info("HALLO")
      sender ! HttpResponse(entity = "HALLO")

    case HttpRequest(HttpMethods.GET, Uri.Path("/status"), _, _, _) =>
      val statusFiles = connections.keys.map { id =>
        val files = dataDir.listFiles
          .filter(f => f.getName.matches(s"message_${id}_\\d+\\.ogg"))
          .sortBy(_.getName)
        (id, status.get(id), files)
      }

      val statusResponse = statusFiles.map {
        case (id, date, files) =>
          s"$id (${date.map(_.toString).getOrElse("never checked")})\n\t${files.map(f => f.getName + " " + f.length()).mkString("\n\t")}"
      }.mkString("\n")

      sender ! HttpResponse(entity = statusResponse)

    case HttpRequest(HttpMethods.GET, Uri.Path(pathId), _, _, _) if pathId.matches("/\\d+") =>
      val id = pathId.substring(1)
      if (dataDir.exists()) {
        log.info(s"GET: $id")
        status += (id -> new Date)
        // try to find the newest file available
        val file = dataDir.listFiles
          .filter(f => f.getName.matches(s"message_${id}_\\d+\\.ogg"))
          .sortBy(_.getName).headOption
        log.info(s"FOUND: $file")

        file match {
          case Some(f) => sender ! HttpResponse(entity = HttpData(f)).withAck(DeleteFile(f))
          case None => sender ! HttpResponse(status = StatusCodes.NotFound)
        }
      } else {
        sender ! HttpResponse(status = StatusCodes.NotFound)
      }

    case HttpRequest(HttpMethods.POST, Uri.Path(pathId), _, entity: HttpEntity.NonEmpty, _) if pathId.matches("/\\d+") =>
      val id = pathId.substring(1)

      dataDir.mkdir()

      log.info(s"POST $id")
      val cal = Calendar.getInstance(TimeZone.getTimeZone("UTC"))
      val df = new SimpleDateFormat("yyyyMMddHHmmss")
      val timestamp: String = df.format(cal.getTimeInMillis)
      val received = s"received_${id}_$timestamp"

      val receivedFile = new File(dataDir, s"$received.tmp")
      val byteWriter = new FileOutputStream(receivedFile)
      byteWriter.write(entity.data.toByteArray)
      byteWriter.flush()
      byteWriter.close()
      log.info(s"SAVED ${receivedFile.getName}")

      val recipients: Seq[String] = connections.getOrElse(id, Seq(id)) match {
        case r if r.isEmpty => Seq(id)
        case r => r
      }
      log.info(s"${recipients}")
      recipients.foreach { recipient =>
        val targetFile = new File(dataDir, s"message_${recipient}_$timestamp.ogg")
        try {
          val result = Files.copy(Paths.get(receivedFile.toURI), Paths.get(targetFile.toURI))
          log.info(s"message: ${result}")
        } catch {
          case e: Exception =>
            log.error(s"can't copy target file: $targetFile", e);
        }
      }
      receivedFile.delete()
      sender ! HttpResponse(status = StatusCodes.OK)

    case _: HttpRequest => sender ! HttpResponse(status = StatusCodes.NotFound)


    case DeleteFile(file) =>
      try {
        if (file.delete)
          log.debug(s"deleted $file")
        else
          log.debug(s"unable to delete $file")
      } catch {
        case e: Exception => println(s"exception deleting $file")
      }

  }
}
