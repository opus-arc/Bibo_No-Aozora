//
//  Bibo_no_AozoraApp.swift
//  Bibo_no-Aozora
//
//  Created by opus arc on 2026/2/5.
//

import SwiftUI
import CoreData

@main
struct Bibo_no_AozoraApp: App {
    let persistenceController = PersistenceController.shared

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(\.managedObjectContext, persistenceController.container.viewContext)
        }
    }
}
