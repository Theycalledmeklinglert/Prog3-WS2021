import { Component, Input } from '@angular/core';
import { Column } from 'src/lib/data-access/modelClasses/Column';

@Component({
  selector: 'board',
  templateUrl: './board.component.html',
  styleUrls: ['./board.component.scss'],
})
export class BoardComponent {
  @Input()
  title = "MyCustomBoard";

  columns: Column[] = [
    {
      id: 0,
      title: "First Column",
      position: 0,
      items: [
        {
          id: 0,
          title: "first task",
          position: 0,
          timestamp: "11-12-2021",
        },
        {
          id: 1,
          title: "second task",
          position: 1,
          timestamp: "11-12-2021",
        },
        {
          id: 2,
          title: "third task",
          position: 2,
          timestamp: "11-12-2021",
        },
      ]
    },

    {
      id: 1,
      title: "Sec Column",
      position: 1,
      items: [
        {
          id: 0,
          title: "first task",
          position: 0,
          timestamp: "11-12-2021",
        },
        {
          id: 1,
          title: "second task",
          position: 1,
          timestamp: "11-12-2021",
        },
        {
          id: 2,
          title: "third task",
          position: 2,
          timestamp: "11-12-2021",
        },
      ]
    },

    {
      id: 2,
      title: "Third Column",
      position: 2,
      items: [
        {
          id: 0,
          title: "first task",
          position: 0,
          timestamp: "11-12-2021",
        },
        {
          id: 1,
          title: "second task",
          position: 1,
          timestamp: "11-12-2021",
        },
        {
          id: 2,
          title: "third task",
          position: 2,
          timestamp: "11-12-2021",
        },
      ]
    },
  ];
}
